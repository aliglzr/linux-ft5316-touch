#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>

#define FT5316_MAX_TOUCH_POINTS	5
#define FT5316_REG_TOUCH_STATUS	0x02
#define FT5316_REG_TOUCH_XH	0x03
#define FT5316_REG_TOUCH_XL	0x04
#define FT5316_REG_TOUCH_YH	0x05
#define FT5316_REG_TOUCH_YL	0x06
#define FT5316_REG_TOUCH_WEIGHT	0x07
#define FT5316_REG_TOUCH_MISC	0x08

struct ft5316_data {
	struct i2c_client *client;
	struct input_dev *input;
	int irq;
	int reset_gpio;
};

static int ft5316_read_reg(struct i2c_client *client, u8 reg, u8 *val)
{
	int ret;

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to read register 0x%02x: %d\n",
			reg, ret);
		return ret;
	}

	*val = ret;
	return 0;
}

static int ft5316_write_reg(struct i2c_client *client, u8 reg, u8 val)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to write register 0x%02x: %d\n",
			reg, ret);
		return ret;
	}

	return 0;
}

static irqreturn_t ft5316_irq_handler(int irq, void *dev_id)
{
	struct ft5316_data *data = dev_id;
	struct i2c_client *client = data->client;
	struct input_dev *input = data->input;
	u8 touch_status;
	u8 touch_data[4];
	int i, ret;

	ret = ft5316_read_reg(client, FT5316_REG_TOUCH_STATUS, &touch_status);
	if (ret < 0)
		return IRQ_HANDLED;

	if (touch_status == 0)
		return IRQ_HANDLED;

	for (i = 0; i < FT5316_MAX_TOUCH_POINTS; i++) {
		ret = i2c_smbus_read_i2c_block_data(client, FT5316_REG_TOUCH_XH,
						    4, touch_data);
		if (ret < 0)
			continue;

		input_mt_slot(input, i);
		input_mt_report_slot_state(input, MT_TOOL_FINGER, true);
		input_report_abs(input, ABS_MT_POSITION_X,
				((touch_data[0] & 0x0f) << 8) | touch_data[1]);
		input_report_abs(input, ABS_MT_POSITION_Y,
				((touch_data[2] & 0x0f) << 8) | touch_data[3]);
	}

	input_mt_sync_frame(input);
	input_sync(input);

	return IRQ_HANDLED;
}

static int ft5316_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct ft5316_data *data;
	struct input_dev *input;
	int error;

	data = devm_kzalloc(&client->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->client = client;
	i2c_set_clientdata(client, data);

	/* Initialize input device */
	input = devm_input_allocate_device(&client->dev);
	if (!input)
		return -ENOMEM;

	input->name = "FT5316 Touchscreen";
	input->id.bustype = BUS_I2C;
	input->dev.parent = &client->dev;

	__set_bit(EV_ABS, input->evbit);
	__set_bit(EV_KEY, input->evbit);
	__set_bit(BTN_TOUCH, input->keybit);

	input_set_abs_params(input, ABS_MT_POSITION_X, 0, 800, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y, 0, 480, 0, 0);
	input_mt_init_slots(input, FT5316_MAX_TOUCH_POINTS, 0);

	data->input = input;

	/* Request IRQ */
	data->irq = client->irq;
	error = devm_request_threaded_irq(&client->dev, data->irq,
					NULL, ft5316_irq_handler,
					IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					"ft5316", data);
	if (error) {
		dev_err(&client->dev, "Failed to request IRQ: %d\n", error);
		return error;
	}

	error = input_register_device(input);
	if (error) {
		dev_err(&client->dev, "Failed to register input device: %d\n",
			error);
		return error;
	}

	return 0;
}

static const struct i2c_device_id ft5316_id[] = {
	{ "ft5316", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ft5316_id);

static const struct of_device_id ft5316_of_match[] = {
	{ .compatible = "focaltech,ft5316", },
	{ }
};
MODULE_DEVICE_TABLE(of, ft5316_of_match);

static struct i2c_driver ft5316_driver = {
	.driver = {
		.name = "ft5316",
		.of_match_table = of_match_ptr(ft5316_of_match),
	},
	.probe = ft5316_probe,
	.id_table = ft5316_id,
};

module_i2c_driver(ft5316_driver);

MODULE_AUTHOR("Ali Golzar");
MODULE_DESCRIPTION("FocalTech FT5316 Touchscreen Driver");
MODULE_LICENSE("GPL v2"); 