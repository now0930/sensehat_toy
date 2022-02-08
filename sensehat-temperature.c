#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

struct hts221 {
	int T0_degC_x8;
	int T1_degC_x8;
	int T0_OUT;
	int T1_OUT;
	int T_OUT;
	int T1T0MSB;
	int temperature;
};

struct rpisense {
	struct device *dev;
	struct i2c_client *i2c_client;
	struct hts221 temperature_data;
	/* Client devices */
};


#define LPS25H (0x5c)	//humidity, temperature
#define LSM9DS1 (0x1c)	//accelometer
#define HTS221 (0x5f)	//humidity, temperature
#define LED2472G (0x46)	//LED Matrix

static struct rpisense *rpisense;

int get_temperature(struct rpisense *rpisense_ptr){
	int ret, littled;
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
	return 0;

}

static int rpisense_probe(struct i2c_client *i2c,
			       const struct i2c_device_id *id)
{
	int ret;
	rpisense = devm_kzalloc(&i2c->dev, sizeof(struct rpisense), GFP_KERNEL);
	if (rpisense == NULL)
		return -ENOMEM;

	i2c_set_clientdata(i2c, rpisense);
	rpisense->dev = &i2c->dev;
	rpisense->i2c_client = i2c;
	pr_info("probed\n");
	get_temperature(rpisense);
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
	rpisense->i2c_client->addr = 0x0;
	return 0;
}

static int rpisense_remove(struct i2c_client *i2c)
{
	struct rpisense *rpisense = i2c_get_clientdata(i2c);
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

