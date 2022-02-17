#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include "sensehat.h"

static int Major = 0;
static int DeviceOpen = 0;
static struct rpisense *rpisense;
static struct class *mychardev_class = NULL;
//frame buffer, global variable
static int frame_buffer[LED_MAX] = {0x00};
int get_temperature(struct rpisense *rpisense_ptr);

//https://tldp.org/LDP/lkmpg/2.6/html/x569.html
//참조
static int mychardev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}


static int device_open(struct inode *inode, struct file *file){
	if (DeviceOpen)
		return -EBUSY;
	DeviceOpen++;
	return 0;


}
static int device_release(struct inode *inode, struct file *file){
	DeviceOpen--;
	return 0;


}
static ssize_t temperature_read(struct file *flip, char *buffer, size_t size, loff_t *offset){
	int temp, ret ;
	size_t len;
	//get temperature
	temp = get_temperature(rpisense);
	sprintf(rpisense->sending_data, "%d\n", temp);
	//send_data = &temp;

	len = sizeof(rpisense->sending_data);
	if (size > len)
		size = len;

	ret = copy_to_user(buffer, (void*)rpisense->sending_data, size);
	if(ret)
		return -EFAULT;
	//pr_info("%s: %d was read\n",__func__, size);
	return size;
}
//https://hyeyoo.com/85
static ssize_t ledmatix_write(struct file *filp, const char *buffer, size_t size, loff_t *offset){
	size_t maxdatalen = LED_MAX * 4;
	int ret;

	if (size < maxdatalen)
		maxdatalen = size; 
	ret = copy_from_user(rpisense->received_image, buffer, maxdatalen);
	
	//databuf[maxdatalen] = 0;
	pr_info("%s: %d remains, %s\n", __func__, ret, rpisense->received_image);
	//pr_info("%s\n", databuf);

	return ret;

}


static struct file_operations tmpre_fops = {
	.read = temperature_read, //a user can read device to get temperature.
	.write = ledmatix_write, // a user can write data to device linked to led matrix
	.open = device_open,
	.release = device_release,
	.read = temperature_read,
	.write = ledmatix_write
};

void clear_display(struct rpisense *rpisense_ptr){
	struct i2c_client *client;
	struct rpisense *rpi;
	int i;
	rpi = rpisense_ptr;
	client = rpi->i2c_client;
	client->addr = LED2472G;
	//pr_info("hit\n");
	for (i=0;i<LED_MAX;i++)
	{
		i2c_smbus_write_byte_data(client, i, 0x00);

	}

}

void set_pixel(int x, int y, int red, int green, int blue){
	int r_addr, g_addr, b_addr;
	//max, min값을 찾기보다 필터 적용
	x = x & 0x7;
	y = y & 0x7;
        r_addr = (y * 24) + x;
        g_addr = r_addr + 8;
        b_addr = g_addr + 8;
	//밝기 최대 63
        frame_buffer[r_addr] = (red & 63);
        frame_buffer[g_addr] = (green & 63);
        frame_buffer[b_addr] = (blue & 63);
}

void flush(struct rpisense *rpisense_ptr){
	struct i2c_client *client;
	struct rpisense *rpi;
	int i;
	rpi = rpisense_ptr;
	client = rpi->i2c_client;
	client->addr = LED2472G;
	for (i=0;i<LED_MAX;i++){
		i2c_smbus_write_byte_data(client, i, frame_buffer[i]);
	}
}

void default_display(struct rpisense *rpisense_ptr){
	struct i2c_client *client;
	struct rpisense *rpi;
	int i;
	rpi = rpisense_ptr;
	client = rpi->i2c_client;
	client->addr = LED2472G;
	for (i=0;i<LED_MAX;i++){
		i2c_smbus_write_byte_data(client, i, default_image[i]);
	}

}

int get_temperature(struct rpisense *rpisense_ptr){
	int ret;
	struct i2c_client *client;
	struct rpisense *rpi;
	rpi = rpisense_ptr;
	client = rpi->i2c_client;
	client->addr = HTS221;
	//0x2a, little endian T_OUT;
	ret = i2c_smbus_read_word_data(client, 0x2a);
	//littled = cpu_to_le16(ret);
	rpi->temperature_data.T_OUT = ret;
	//pr_info("T_OUT: %x\n",ret);

	//pr_info("address: %x\n",client->addr);
	//pr_info("val: %x\n",rpi->temperature_data.T_OUT);
	ret = i2c_smbus_read_byte_data(client, 0x32);
	//ret = cpu_to_le16(ret);
	rpi->temperature_data.T0_degC_x8 = ret;
	//pr_info("T0_degC: %x\n",ret);

	ret = i2c_smbus_read_byte_data(client, 0x33);
	//littled = cpu_to_le16(ret);
	rpi->temperature_data.T1_degC_x8 = ret;
	//pr_info("T1_degC: %x\n",ret);

	//T1T0 msb
	ret = i2c_smbus_read_byte_data(client, 0x35);
	rpi->temperature_data.T1T0MSB = ret;

	rpi->temperature_data.T0_degC_x8 = \
		(rpi->temperature_data.T0_degC_x8 + (1<<8)*(ret & 0x03))/8;
	//pr_info("T0_degC: %x\n",rpi->temperature_data.T0_degC_x8);

	rpi->temperature_data.T1_degC_x8 = \
		(rpi->temperature_data.T1_degC_x8 + (1<<6)*(ret & 0x0c))/8;
	//pr_info("T1_degC: %x\n",rpi->temperature_data.T1_degC_x8);

	ret = i2c_smbus_read_word_data(client, 0x3c);
	//littled = cpu_to_le16(ret);
	rpi->temperature_data.T0_OUT = ret;
	//pr_info("T0_OUT: %x\n",ret);

	ret = i2c_smbus_read_word_data(client, 0x3e);
	//littled = cpu_to_le16(ret);
	rpi->temperature_data.T1_OUT = ret;
	//pr_info("T1_OUT: %x\n",ret);


	//measeure temperature
	ret = rpi->temperature_data.T0_degC_x8 + \
	      (rpi->temperature_data.T_OUT - rpi->temperature_data.T0_OUT) *\
	      (rpi->temperature_data.T1_degC_x8 - rpi->temperature_data.T0_degC_x8)\
	      /(rpi->temperature_data.T1_OUT - rpi->temperature_data.T0_OUT);

	rpi->temperature_data.temperature = ret;
	//pr_info("val: %x\n",ret);
	if ((ret < 0) | (ret > 40))
		return -1;
	else
		return ret;

}


static int rpisense_probe(struct i2c_client *i2c,
			       const struct i2c_device_id *id)
{
	int ret;
	dev_t dev;
	rpisense = devm_kzalloc(&i2c->dev, sizeof(struct rpisense), GFP_KERNEL);
	if (rpisense == NULL)
		return -ENOMEM;

	i2c_set_clientdata(i2c, rpisense);
	rpisense->dev = &i2c->dev;
	rpisense->i2c_client = i2c;

	//https://olegkutkov.me/2018/03/14/simple-linux-character-device-driver/
	ret = alloc_chrdev_region(&dev, 0, 1, "char_tmpre");
	Major = MAJOR(dev);
	
	//user permission configure.
	//https://olegkutkov.me/2018/03/14/simple-linux-character-device-driver/
	mychardev_class = class_create(THIS_MODULE, "rpi-sense-tmpre");
	mychardev_class->dev_uevent = mychardev_uevent;

	cdev_init(&rpisense->cdev_temperature, &tmpre_fops);
	rpisense->cdev_temperature.owner = THIS_MODULE;

	cdev_add(&rpisense->cdev_temperature, MKDEV(Major, 0), 1);

	device_create(mychardev_class, NULL, MKDEV(Major, 0), NULL, "rs-tmpre%d",1);

	pr_info(KERN_INFO "%d was reigistered\n",Major);
	if (Major < 0){
		pr_info(KERN_ALERT "Registering char device was failed\n");
		return Major;
	}

	

	pr_info("probed\n");
	pr_info("temperature: %d\n",get_temperature(rpisense));
	//display address is 0x46
	//pr_info("client address is: %x\n", rpisense->i2c_client->addr);
	//rpisense->i2c_client->addr = HTS221;
	///pr_info("client address is: %x\n", rpisense->i2c_client->addr);
	//pr_info("client id name is: %s\n", id->name);
	//read temperature data
	//who am i 
	//ret=i2c_smbus_read_byte_data(rpisense->i2c_client, 0x0f);
	//littled = cpu_to_le16(ret);
	//pr_info("my name is %x\n",littled);
	//littled = cpu_to_le16(ret);
	//ret=i2c_smbus_read_byte_data(rpisense->i2c_client, 0x2a);
	//pr_info("read: %d\n",littled);
	//clear_display(rpisense);
	default_display(rpisense);
	//set_pixel(2,0, 0x00, 0x1, 0x0);
	//set_pixel(3,1, 0x00, 0x0, 0x1);
	//flush(rpisense);
	rpisense->i2c_client->addr = 0x0;
	return 0;
}

static int rpisense_remove(struct i2c_client *i2c)
{
	struct rpisense *rpisense = i2c_get_clientdata(i2c);
	device_destroy(mychardev_class, MKDEV(Major, 0));
	class_unregister(mychardev_class);
	//i2c remove가 destroy
	//class_destroy(mychardev_class);
	unregister_chrdev_region(MKDEV(Major, 0), 1);
	//unregister_chrdev(Major, DeviceName);
	pr_info(KERN_INFO "unregistering device\n");
	clear_display(rpisense);
	pr_info("removed\n");
	return 0;
}



static const struct i2c_device_id rpisense_i2c_id[] = {
	{ "rpi-sense", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, rpisense_i2c_id);

static const struct of_device_id rpisense_core_id[] = {
	{ .compatible = "rpi,rpi-sense" },
	{ },
};
MODULE_DEVICE_TABLE(of, rpisense_core_id);


static struct i2c_driver rpisense_driver = {
	.driver = {
		   .name = "rpi-sense",
		   .owner = THIS_MODULE,
	},
	.probe = rpisense_probe,
	.remove = rpisense_remove,
	.id_table = rpisense_i2c_id,
};

module_i2c_driver(rpisense_driver);

MODULE_DESCRIPTION("Raspberry Pi Sense HAT core driver");
MODULE_AUTHOR("Serge Schneider <serge@raspberrypi.org>");
MODULE_LICENSE("GPL");

