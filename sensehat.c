#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include "sensehat.h"

static int Major = 0;
static int DeviceOpen = 0;
static int H_DeviceOpen = 0;
static struct rpisense *rpisense;
static struct class *mychardev_class = NULL;
//frame buffer, global variable
static int frame_buffer[LED_MAX] = {0x00};

//https://tldp.org/LDP/lkmpg/2.6/html/x569.html
//참조
static int mychardev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}


static int temper_device_open(struct inode *inode, struct file *file){
	if (DeviceOpen)
		return -EBUSY;
	DeviceOpen++;
	//pr_info("opened\n");
	return 0;


}
static int temper_device_release(struct inode *inode, struct file *file){
	//pr_info("released\n");
	DeviceOpen--;
	return 0;
}
static ssize_t temperature_read(struct file *flip, char *buffer, size_t size, loff_t *offset){
	int temp, ret ;
	size_t len;
	struct hts221* my;
	my = &rpisense->hts221_data;
	//get temperature
	temp = get_hts221(rpisense);
	//pr_info("%s: %d\n",__func__, temp);
	sprintf(rpisense->sending_temperature + strlen(rpisense->sending_temperature), "%d,", temp);
	sprintf(rpisense->sending_temperature + strlen(rpisense->sending_temperature), "%d,", my->T0_degC_x8);
	sprintf(rpisense->sending_temperature + strlen(rpisense->sending_temperature), "%d,", my->T1_degC_x8);
	sprintf(rpisense->sending_temperature + strlen(rpisense->sending_temperature), "%d,", my->T0_OUT);
	sprintf(rpisense->sending_temperature + strlen(rpisense->sending_temperature), "%d,", my->T1_OUT);
	sprintf(rpisense->sending_temperature + strlen(rpisense->sending_temperature), "%d,", my->T_OUT);
	sprintf(rpisense->sending_temperature + strlen(rpisense->sending_temperature), "%d", my->T1T0MSB);

	//send_data = &temp;

	len = sizeof(rpisense->sending_temperature);

	if (size > len)
		size = len;

	ret = copy_to_user(buffer, (void*)rpisense->sending_temperature, size);
	if(ret)
		return -EFAULT;
	//pr_info("%s: %d was read\n",__func__, size);
	sprintf(rpisense->sending_temperature, "%s","");
	return size;
}
//https://hyeyoo.com/85
static ssize_t temper_ledmat_write(struct file *filp, const char *buffer, size_t size, loff_t *offset){
	size_t maxdatalen = LED_MAX;
	int ret;

	if (size < maxdatalen)
		maxdatalen = size; 
	ret = copy_from_user(rpisense->recv_temp_image, buffer, maxdatalen);
	
	//databuf[maxdatalen] = 0;
	//pr_info("%s: %d remains, %s\n", __func__, ret, rpisense->recv_temp_image);
	//pr_info("%s: %d written\n", __func__, sizeof(rpisense->recv_temp_image));
	schedule_work(&rpisense->temp_queue);
	return ret;

}

static int humidy_device_open(struct inode *inode, struct file *file){
	if (H_DeviceOpen)
		return -EBUSY;
	H_DeviceOpen++;
	//pr_info("humidity opened\n");
	return 0;
}

static int humidy_device_release(struct inode *inode, struct file *file){
	//pr_info("humidity released\n");
	H_DeviceOpen--;
	return 0;
}

static ssize_t humidy_read(struct file *flip, char *buffer, size_t size, loff_t *offset){

	int ret ;
	size_t len;
	struct hts221* my;
	my = &rpisense->hts221_data;
	//get temperature
	get_hts221(rpisense);
	//pr_info("%s: %d\n",__func__, temp);
	sprintf(rpisense->sending_humidity + strlen(rpisense->sending_humidity), "%d,", my->H0_rH_x2);
	sprintf(rpisense->sending_humidity + strlen(rpisense->sending_humidity), "%d,", my->H1_rH_x2);
	sprintf(rpisense->sending_humidity + strlen(rpisense->sending_humidity), "%d,", my->H0_T0_OUT);
	sprintf(rpisense->sending_humidity + strlen(rpisense->sending_humidity), "%d,", my->H1_T0_OUT);
	sprintf(rpisense->sending_humidity + strlen(rpisense->sending_humidity), "%d", my->H_OUT);

	len = sizeof(rpisense->sending_humidity);
	//pr_info("%s: %d length\n", __func__, len);
	//pr_info("%s: %s\n", __func__, rpisense->sending_humidity);

	if (size > len)
		size = len;

	ret = copy_to_user(buffer, (void*)rpisense->sending_humidity, size);
	if(ret)
		return -EFAULT;
	//pr_info("%s: %d was read\n",__func__, size);
	sprintf(rpisense->sending_humidity, "%s","");
	return size;
}

static ssize_t humidy_ledmatix_write(struct file *filp, const char *buffer, size_t size, loff_t *offset){
	size_t maxdatalen = LED_MAX;
	int ret;

	if (size < maxdatalen)
		maxdatalen = size; 
	ret = copy_from_user(rpisense->recv_humi_image, buffer, maxdatalen);

	//databuf[maxdatalen] = 0;
	schedule_work(&rpisense->humi_queue);
	return ret;
}


static struct file_operations tmpre_fops = {
	.read = temperature_read, //a user can read device to get temperature.
	.write = temper_ledmat_write, // a user can write data to device linked to led matrix
	.open = temper_device_open,
	.release = temper_device_release,
};


static struct file_operations humidy_fops= {
	.read = humidy_read, //a user can read device to get humidity.
	.write = humidy_ledmatix_write, // a user can write data to device linked to led matrix to display humidity
	.open = humidy_device_open,
	.release = humidy_device_release,
};

static void workfn_display(struct work_struct *work){
	//pr_info("work queue was activeted\n");
	//display temperature
	flush(rpisense, false);
}

static void workfn_display2(struct work_struct *work){
	//pr_info("work queue was activeted\n");
	//display temperature
	flush(rpisense, true);
}


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

void flush(struct rpisense *rpisense_ptr, bool flag){
	struct i2c_client *client;
	struct rpisense *rpi;
	int i;
	rpi = rpisense_ptr;
	client = rpi->i2c_client;
	client->addr = LED2472G;
	if ( flag == true){
		for (i=0;i<LED_MAX;i++){
			//i2c_smbus_write_byte_data(client, i, frame_buffer[i]);
			i2c_smbus_write_byte_data(client, i, rpi->recv_temp_image[i]);

		}
	}
	else{
		for (i=0;i<LED_MAX;i++){
			//i2c_smbus_write_byte_data(client, i, frame_buffer[i]);
			i2c_smbus_write_byte_data(client, i, rpi->recv_humi_image[i]);

		}
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

void setup_hts221(struct rpisense *rpisense_ptr){
	struct i2c_client *client;
	struct rpisense *rpi;
	rpi = rpisense_ptr;
	client = rpi->i2c_client;
	client->addr = HTS221;

	//0x20, power on, ...
	//device didnt update temperature after loading module.
	//i2c_smbus_write_byte_data(client, 0x20, 0x81);
	i2c_smbus_write_byte_data(client, 0x20, 0x87);
	//oneshot initialize
	i2c_smbus_write_byte_data(client, 0x21, 0x0);
}

void shutdown_hts221(struct rpisense *rpisense_ptr){
	struct i2c_client *client;
	struct rpisense *rpi;
	rpi = rpisense_ptr;
	client = rpi->i2c_client;
	client->addr = HTS221;
	i2c_smbus_write_byte_data(client, 0x20, 0x00);


}


int get_hts221(struct rpisense *rpisense_ptr){
	//읽을 때 워드로 한번에 읽으면 안 읽어짐.
	//바이트로 읽어 8비트씩 시프트 해야 함.
	//https://blog.artwolf.in/a?ID=3d68fd7b-3948-45f9-816f-697ce0397d5e
	int ret;
	int temp;
	struct i2c_client *client;
	struct rpisense *rpi;
	rpi = rpisense_ptr;

	client = rpi->i2c_client;
	client->addr = HTS221;

	//0x27 chech..ready bit
	ret = i2c_smbus_read_word_data(client, 0x27);
	if (!(ret & 0x3)){
		pr_info(KERN_ERR "register was not ready\n");
		return 0;
	}

	//0x2a, T_OUT

	ret = i2c_smbus_read_byte_data(client, 0x2a);
	temp = i2c_smbus_read_byte_data(client, 0x2b);
	rpi->hts221_data.T_OUT = (((uint16_t)(temp << 8)) | ((uint16_t)ret));

	/*
	ret = i2c_smbus_read_word_data(client, 0x2a);
	rpi->hts221_data.T_OUT = ret;
	*/


	//0x28, H_OUT
	ret = i2c_smbus_read_byte_data(client, 0x28);
	temp = i2c_smbus_read_byte_data(client, 0x29);
	rpi->hts221_data.H_OUT = (((uint16_t)(temp << 8)) | ((uint16_t)ret));

	//0x30, H0_rH_x2
	ret = i2c_smbus_read_byte_data(client, 0x30);
	rpi->hts221_data.H0_rH_x2= ret>>1;
	//pr_info("rpi->hts221.H0_rH_x2: %d\n",rpi->hts221_data.H0_rH_x2);

	//0x31, H1_rH_x2
	ret = i2c_smbus_read_byte_data(client, 0x31);
	rpi->hts221_data.H1_rH_x2= ret>>1;
	//pr_info("rpi->hts221.H1_rH_x2: %d\n",rpi->hts221_data.H1_rH_x2);

	//0x32, T0_degC_x8
	ret = i2c_smbus_read_byte_data(client, 0x32);
	rpi->hts221_data.T0_degC_x8 = ret;

	//0x33, T1_degC_x8
	ret = i2c_smbus_read_byte_data(client, 0x33);
	rpi->hts221_data.T1_degC_x8 = ret;

	//0x35, T1T0 msb
	ret = i2c_smbus_read_byte_data(client, 0x35);
	rpi->hts221_data.T1T0MSB = ret;

	rpi->hts221_data.T0_degC_x8 = \
		(rpi->hts221_data.T0_degC_x8 + (1<<8)*(ret & 0x03))/8;

	rpi->hts221_data.T1_degC_x8 = \
		(rpi->hts221_data.T1_degC_x8 + (1<<6)*(ret & 0x0c))/8;

	//0x36, H0_T0_OUT
	ret = i2c_smbus_read_byte_data(client, 0x36);
	temp = i2c_smbus_read_byte_data(client, 0x37);
	rpi->hts221_data.H0_T0_OUT = (((uint16_t)(temp << 8)) | ((uint16_t)ret));
	//pr_info("rpi->hts221.H0_T0_OUT: %d\n",rpi->hts221_data.H0_T0_OUT);


	//0x3a,  H1_T0_OUT
	ret = i2c_smbus_read_byte_data(client, 0x3a);
	temp = i2c_smbus_read_byte_data(client, 0x3b);
	rpi->hts221_data.H1_T0_OUT = (((uint16_t)(temp << 8)) | ((uint16_t)ret));
	//pr_info("rpi->hts221.H1_T0_OUT: %d\n",rpi->hts221_data.H1_T0_OUT);


	//0x3c, T0_OUT

	ret = i2c_smbus_read_byte_data(client, 0x3c);
	temp = i2c_smbus_read_byte_data(client, 0x3d);
	rpi->hts221_data.T0_OUT = (((uint16_t)(temp << 8)) | ((uint16_t)ret));
	//pr_info(KERN_ERR "ret: %0x\n", ret);
	//pr_info(KERN_ERR "temp: %0x\n", temp);

 
	/*
	ret = i2c_smbus_read_word_data(client, 0x3c);
	rpi->hts221_data.T0_OUT = ret;
	*/
	//0x3e, T1_OUT

	ret = i2c_smbus_read_byte_data(client, 0x3e);
	temp = i2c_smbus_read_byte_data(client, 0x3f);
	rpi->hts221_data.T1_OUT = (((uint16_t)(temp << 8)) | ((uint16_t)ret));

	/*
 	ret = i2c_smbus_read_word_data(client, 0x3e);
	rpi->hts221_data.T1_OUT = ret;
	*/
	//measeure temperature
	ret = rpi->hts221_data.T0_degC_x8 + \
	      (rpi->hts221_data.T_OUT - rpi->hts221_data.T0_OUT) *\
	      (rpi->hts221_data.T1_degC_x8 - rpi->hts221_data.T0_degC_x8)\
	      /(rpi->hts221_data.T1_OUT - rpi->hts221_data.T0_OUT);

	rpi->hts221_data.temperature = ret;

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
	ret = alloc_chrdev_region(&dev, 0, 2, "char_tmpre");
	Major = MAJOR(dev);
	
	//user permission configure.
	//https://olegkutkov.me/2018/03/14/simple-linux-character-device-driver/
	mychardev_class = class_create(THIS_MODULE, "rpi-sensehat");
	mychardev_class->dev_uevent = mychardev_uevent;

	cdev_init(&rpisense->cdev_temperature, &tmpre_fops);
	cdev_init(&rpisense->cdev_humidity, &humidy_fops);
	rpisense->cdev_temperature.owner = THIS_MODULE;

	cdev_add(&rpisense->cdev_temperature, MKDEV(Major, 0), 1);
	cdev_add(&rpisense->cdev_humidity, MKDEV(Major, 1), 1);

	device_create(mychardev_class, NULL, MKDEV(Major, 0), NULL, "rs-tmpre%d",1);
	device_create(mychardev_class, NULL, MKDEV(Major, 1), NULL, "rs-tmpre%d",2);

	pr_info(KERN_INFO "%d was reigistered\n",Major);
	if (Major < 0){
		pr_info(KERN_ALERT "Registering char device was failed\n");
		return Major;
	}


	setup_hts221(rpisense);
	pr_info("probed\n");
	default_display(rpisense);
	INIT_WORK(&rpisense->temp_queue, workfn_display);
	INIT_WORK(&rpisense->humi_queue, workfn_display2);

	rpisense->i2c_client->addr = 0x0;
	return 0;
}

static int rpisense_remove(struct i2c_client *i2c)
{
	struct rpisense *rpisense = i2c_get_clientdata(i2c);
	device_destroy(mychardev_class, MKDEV(Major, 0));
	device_destroy(mychardev_class, MKDEV(Major, 1));
	class_unregister(mychardev_class);
	//i2c remove가 destroy
	//class_destroy(mychardev_class);
	unregister_chrdev_region(MKDEV(Major, 0), 2);
	//unregister_chrdev(Major, DeviceName);
	pr_info(KERN_INFO "unregistering device\n");
	flush_work(&rpisense->temp_queue);
	flush_work(&rpisense->humi_queue);
	clear_display(rpisense);
	shutdown_hts221(rpisense);
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
MODULE_AUTHOR("now0930");
MODULE_LICENSE("GPL");
