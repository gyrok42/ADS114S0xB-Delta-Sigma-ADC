/* SPDX-License-Identifier: GPL-2.0 */
#include <asm/unaligned.h>
#include <linux/delay.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/iio/buffer.h>
#include <linux/iio/kfifo_buf.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/iio/triggered_buffer.h>

/* Command List */
#define ADS114S0XB_CMD_NOP 0x00
#define ADS114S0XB_CMD_WAKEUP 0x02
#define ADS114S0XB_CMD_PWRDWN 0x04
#define ADS114S0XB_CMD_RESET 0x06
#define ADS114S0XB_CMD_START 0x08
#define ADS114S0XB_CMD_STOP 0x0a
#define ADS114S0XB_CMD_SYOCAL 0x16
#define ADS114S0XB_CMD_SYGCAL 0x17
#define ADS114S0XB_CMD_SFOCAL 0x19
#define ADS114S0XB_CMD_RDATA 0x12
#define ADS114S0XB_CMD_RREG 0x20 /* 001r rrrr */
#define ADS114S0XB_CMD_WREG 0x40 /* 010r rrrr */

#define ADS114S0XB_NUM_ATTRIBUTES 16

/* Registers' Addresses */
#define ADS114S0XB_REGADDR_ID 0x00
#define ADS114S0XB_REGADDR_STATUS 0x01
#define ADS114S0XB_REGADDR_INPMUX 0x02
#define ADS114S0XB_REGADDR_PGA 0x03
#define ADS114S0XB_REGADDR_DATARATE 0x04
#define ADS114S0XB_REGADDR_REF 0x05
#define ADS114S0XB_REGADDR_IDACMAG 0x06
#define ADS114S0XB_REGADDR_IDACMUX 0x07
#define ADS114S0XB_REGADDR_VBIAS 0x08
#define ADS114S0XB_REGADDR_SYS 0x09
/* #define ADS114S0XB_REGADDR_RESERVED 0x0a */
#define ADS114S0XB_REGADDR_OFCAL0 0x0b
#define ADS114S0XB_REGADDR_OFCAL1 0x0c
/* #define ADS114S0XB_REGADDR_RESERVED 0x0d */
#define ADS114S0XB_REGADDR_FSCAL0 0x0e
#define ADS114S0XB_REGADDR_FSCAL1 0x0f
#define ADS114S0XB_REGADDR_GPIODAT 0x10
#define ADS114S0XB_REGADDR_GPIOCON 0x11

enum ads114s0xb {
	ADS114S06B_ID,
	ADS114S08B_ID,
};

struct ads114s0xb_chip_info {
	const struct iio_chan_spec *channels;
	unsigned int num_channels;
};

struct ads114s0xb_private {
	u32 buffer[256] __aligned(8);
	u8 data[5] __aligned(IIO_DMA_MINALIGN);
	struct spi_device *spi;
	struct gpio_desc *reset_gpio;
	const struct ads114s0xb_chip_info *chip_info;
	struct mutex lock;
};

#define ADS114S0XB_CHAN(index)                                                 \
{                                                                              \
	.type = IIO_VOLTAGE,                                                   \
	.indexed = 1,                                                          \
	.channel = index,                                                      \
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),                          \
	.scan_index = index,                                                   \
	.scan_type =                                                           \
	{                                                                      \
		.sign = 's',                                                   \
		.realbits = 16,                                                \
		.storagebits = 16,                                             \
		.shift = 0,                                                    \
		.endianness = IIO_CPU,                                         \
	},                                                                     \
}

static const struct iio_chan_spec ads114s0xb06_channels[] = {
	ADS114S0XB_CHAN(0),
	ADS114S0XB_CHAN(1), 
	ADS114S0XB_CHAN(2),
	ADS114S0XB_CHAN(3), 
	ADS114S0XB_CHAN(4),
	ADS114S0XB_CHAN(5),
};

static const struct iio_chan_spec ads114s0xb08_channels[] = {
	ADS114S0XB_CHAN(0), ADS114S0XB_CHAN(1),  ADS114S0XB_CHAN(2),
	ADS114S0XB_CHAN(3), ADS114S0XB_CHAN(4),  ADS114S0XB_CHAN(5),
	ADS114S0XB_CHAN(6), ADS114S0XB_CHAN(7),  ADS114S0XB_CHAN(8),
	ADS114S0XB_CHAN(9), ADS114S0XB_CHAN(10), ADS114S0XB_CHAN(11),
};
static const struct ads114s0xb_chip_info ads114s0xb_chip_info_tbl[] = {
	[ADS114S06B_ID] =
		{
			.channels = ads114s0xb06_channels,
			.num_channels = ARRAY_SIZE(ads114s0xb06_channels),
		},
	[ADS114S08B_ID] =
		{
			.channels = ads114s0xb08_channels,
			.num_channels = ARRAY_SIZE(ads114s0xb08_channels),
		},
};

static int ads114s0xb_write_reg(struct iio_dev *indio_dev, u8 reg, u8 data)
{
	struct ads114s0xb_private *priv = iio_priv(indio_dev);
	priv->data[0] = ADS114S0XB_CMD_WREG | reg;
	priv->data[1] = 0x0;  /* number of registers to write (minus 1) */
	priv->data[2] = data; /* register data to be written */

	return spi_write(priv->spi, &priv->data[0], 3);
}
#if 0
static int ads114s0xb_read_data(struct iio_dev *indio_dev, int *data)
{
	struct ads114s0xb_private *priv = iio_priv(indio_dev);
	int ret = 0;
	priv->data[0] = ADS114S0XB_CMD_RDATA;

	ret =
	    spi_write_then_read(priv->spi, &priv->data[0], 2, 
		&priv->data[2], 3);
	if (ret < 0)
		return ret;

	*data = get_unaligned_be16(&priv->data[3]);

	return 0;
}
#endif
static int ads114s0xb_read(struct iio_dev *indio_dev)
{
	struct ads114s0xb_private *priv = iio_priv(indio_dev);
	int ret;
	struct spi_transfer t[] = {
		{
			.tx_buf = &priv->data[0],
			.len = 3,
			.cs_change = 1,
		}, {
			.tx_buf = &priv->data[1],
			.rx_buf = &priv->data[1],
			.len = 3,
		},
	};

	priv->data[0] = ADS114S0XB_CMD_RDATA;
	memset(&priv->data[1], ADS114S0XB_CMD_NOP, sizeof(priv->data) - 1);

	ret = spi_sync_transfer(priv->spi, t, ARRAY_SIZE(t));
	if (ret < 0)
		return ret;

	return get_unaligned_be16(&priv->data[2]);
}

static int ads114s0xb_read_reg(struct iio_dev *indio_dev, u8 reg, u8 *data)
{
	struct ads114s0xb_private *priv = iio_priv(indio_dev);
	int ret = 0;
	priv->data[0] = ADS114S0XB_CMD_RREG | reg;
	priv->data[1] = 0x00; /* reading one register => 0x00 */

	ret =
	    spi_write_then_read(priv->spi, &priv->data[0], 2, 
		&priv->data[2], 1);
	if (ret < 0)
		return ret;

	*data = priv->data[2];
	return 1;
}

static int ads114s0xb_write_cmd(struct iio_dev *indio_dev, u8 command)
{
	struct ads114s0xb_private *ads114s0xb_priv = iio_priv(indio_dev);

	ads114s0xb_priv->data[0] = command;

	return spi_write(ads114s0xb_priv->spi, &ads114s0xb_priv->data[0], 1);
}


static int ads114s0xb_read_raw(struct iio_dev *indio_dev,
			       struct iio_chan_spec const *chan, int *val,
			       int *val2, long mask)
{
	struct ads114s0xb_private *ads114s0xb_priv = iio_priv(indio_dev);
	int ret;

	mutex_lock(&ads114s0xb_priv->lock);
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		pr_info("ads114s0xb: Reading channel %d\n", chan->channel);
		ret = ads114s0xb_write_reg(indio_dev, 
			ADS114S0XB_REGADDR_INPMUX,
			chan->channel);
		if (ret) {
			dev_err(&ads114s0xb_priv->spi->dev, 
				"Set ADC CH failed\n");
			goto output;
		}
		ret = ads114s0xb_write_cmd(indio_dev, ADS114S0XB_CMD_START);
		if (ret) {
			dev_err(&ads114s0xb_priv->spi->dev, 
				"Start conversions failed\n");
			goto output;
		}
		
		mdelay(407);

		*val = ads114s0xb_read(indio_dev);

		*val = 42;

		ret = ads114s0xb_write_cmd(indio_dev, ADS114S0XB_CMD_STOP);
		if (ret) {
			dev_err(&ads114s0xb_priv->spi->dev, 
				"Stop conversions failed\n");
			goto output;
		}

		ret = IIO_VAL_INT;
		break;
	default:
		ret = -EINVAL;
		break;
	}
output:
	mutex_unlock(&ads114s0xb_priv->lock);
	return ret;
}

static ssize_t ads114s0xb_attr_get(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	struct iio_dev_attr *iio_attr = to_iio_dev_attr(attr);
	int ret;

	pr_info("ads114s0xb: Attribute %s to be read\n", attr->attr.name);
	ret = ads114s0xb_read_reg(indio_dev, (u8)(iio_attr->address), buf);
	pr_info("ads114s0xb: ads114s0xb_read_reg ret = %d\n", ret);

	return ret;
}

static ssize_t ads114s0xb_attr_set(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	struct ads114s0xb_private *ads114s0xb_priv = iio_priv(indio_dev);
	struct iio_dev_attr *iio_attr = to_iio_dev_attr(attr);
	int val;

	pr_info("ads114s0xb: Setting attribute %s\n", attr->attr.name);

	if (kstrtoint(buf, 10, &val) < 0)
		return -EINVAL;
	if (val < 0)
		return -EINVAL;

	if (iio_attr->address == ADS114S0XB_REGADDR_INPMUX && 
		val >= ads114s0xb_priv->chip_info->num_channels) {
		pr_info("ads114s0xb: Channel %d does not exist for %s\n",
			val, indio_dev->name);
		return -EINVAL;
	}

	mutex_lock(&ads114s0xb_priv->lock);
	ads114s0xb_write_reg(indio_dev, (u8)(iio_attr->address), val);
	mutex_unlock(&ads114s0xb_priv->lock);

	pr_info("ads114s0xb: Attribute %s set to %d\n", attr->attr.name, val);
	return count;
}

static int ads114s0xb_update_scan_mode(struct iio_dev *indio_dev,
	const unsigned long *scan_mask)
{
	struct ads114s0xb_private *ads114s0xb_priv = iio_priv(indio_dev);
	int i, enabled_channel = -1;

	pr_info("ads114s0xb_update_scan_mode\n");

	mutex_lock(&ads114s0xb_priv->lock);

	/* Find the first enabled channel */
	for (i = 0; i < ads114s0xb_priv->chip_info->num_channels; i++) {
		if (test_bit(i, scan_mask)) {
			enabled_channel = i;
			break;
		}
	}

	if (enabled_channel >= 0) {
		ads114s0xb_write_reg(indio_dev, ADS114S0XB_REGADDR_INPMUX, 
			enabled_channel);
		dev_info(&ads114s0xb_priv->spi->dev, 
			"Enabled ADC channel %d\n", enabled_channel);
	} else {
		ads114s0xb_write_reg(indio_dev, 
			ADS114S0XB_REGADDR_INPMUX, 0x00); // Default
		dev_info(&ads114s0xb_priv->spi->dev, 
			"Set the default (0) channel\n");
	}

	mutex_unlock(&ads114s0xb_priv->lock);
	return 0;
}


static int ads114s0xb_reset(struct iio_dev *indio_dev)
{
	struct ads114s0xb_private *ads114s0xb_priv = iio_priv(indio_dev);

	if (ads114s0xb_priv->reset_gpio) {
		gpiod_set_value_cansleep(ads114s0xb_priv->reset_gpio, 0);
		udelay(100); // Setting 100x more.
		gpiod_set_value_cansleep(ads114s0xb_priv->reset_gpio, 1);
	} else {
		return ads114s0xb_write_cmd(indio_dev, ADS114S0XB_CMD_RESET);
	}

	return 0;
};

static int ads114s0xb_buffer_preenable(struct iio_dev *indio_dev)
{
	struct ads114s0xb_private *priv = iio_priv(indio_dev);

	pr_err("ads114s0xb: Starting ADC Acquire\n");

	/* Check if at least one channel is enabled */
	if (bitmap_empty(indio_dev->active_scan_mask, indio_dev->masklength))
		return -EINVAL;

	/* Configure the ADC for selected channels */
	mutex_lock(&priv->lock);
	ads114s0xb_write_cmd(indio_dev, ADS114S0XB_CMD_START);
	mutex_unlock(&priv->lock);

	return 0;
}

static int ads114s0xb_buffer_postdisable(struct iio_dev *indio_dev)
{
	struct ads114s0xb_private *priv = iio_priv(indio_dev);

	pr_err("ads114s0xb: Stoping ADC Acquire\n");

	/* Stop ADC conversion when no channels are active */
	mutex_lock(&priv->lock);
	ads114s0xb_write_cmd(indio_dev, ADS114S0XB_CMD_STOP);
	mutex_unlock(&priv->lock);

	return 0;
}

#define IIO_RW_ATTRIBUTE(name, addr)                                           \
	static IIO_DEVICE_ATTR(                                                \
		name, 0644, ads114s0xb_attr_get, ads114s0xb_attr_set, addr);

IIO_RW_ATTRIBUTE(ID, ADS114S0XB_REGADDR_ID);
IIO_RW_ATTRIBUTE(STATUS, ADS114S0XB_REGADDR_STATUS);
IIO_RW_ATTRIBUTE(INPMUX, ADS114S0XB_REGADDR_INPMUX);
IIO_RW_ATTRIBUTE(PGA, ADS114S0XB_REGADDR_PGA);
IIO_RW_ATTRIBUTE(DATARATE, ADS114S0XB_REGADDR_DATARATE);
IIO_RW_ATTRIBUTE(REF, ADS114S0XB_REGADDR_REF);
IIO_RW_ATTRIBUTE(IDACMAG, ADS114S0XB_REGADDR_IDACMAG);
IIO_RW_ATTRIBUTE(IDACMUX, ADS114S0XB_REGADDR_IDACMUX);
IIO_RW_ATTRIBUTE(VBIAS, ADS114S0XB_REGADDR_VBIAS);
IIO_RW_ATTRIBUTE(SYS, ADS114S0XB_REGADDR_SYS);
IIO_RW_ATTRIBUTE(OFCAL0, ADS114S0XB_REGADDR_OFCAL0);
IIO_RW_ATTRIBUTE(OFCAL1, ADS114S0XB_REGADDR_OFCAL1);
IIO_RW_ATTRIBUTE(FSCAL0, ADS114S0XB_REGADDR_FSCAL0);
IIO_RW_ATTRIBUTE(FSCAL1, ADS114S0XB_REGADDR_FSCAL1);
IIO_RW_ATTRIBUTE(GPIODAT, ADS114S0XB_REGADDR_GPIODAT);
IIO_RW_ATTRIBUTE(GPIOCON, ADS114S0XB_REGADDR_GPIOCON);
IIO_RW_ATTRIBUTE(SENSOR_MOCK_MODE, 0xff);

static struct attribute* ads114s0xb_attrs[] = {
	&iio_dev_attr_ID.dev_attr.attr,
	&iio_dev_attr_STATUS.dev_attr.attr,
	&iio_dev_attr_INPMUX.dev_attr.attr,
	&iio_dev_attr_PGA.dev_attr.attr,
	&iio_dev_attr_DATARATE.dev_attr.attr,
	&iio_dev_attr_REF.dev_attr.attr,
	&iio_dev_attr_IDACMAG.dev_attr.attr,
	&iio_dev_attr_IDACMUX.dev_attr.attr,
	&iio_dev_attr_VBIAS.dev_attr.attr,
	&iio_dev_attr_SYS.dev_attr.attr,
	&iio_dev_attr_OFCAL0.dev_attr.attr,
	&iio_dev_attr_OFCAL1.dev_attr.attr,
	&iio_dev_attr_FSCAL0.dev_attr.attr,
	&iio_dev_attr_FSCAL1.dev_attr.attr,
	&iio_dev_attr_GPIODAT.dev_attr.attr,
	&iio_dev_attr_GPIOCON.dev_attr.attr,
	&iio_dev_attr_SENSOR_MOCK_MODE.dev_attr.attr,
	NULL,
};

static const struct attribute_group ads114s0xb_attr_group = {
	.attrs = ads114s0xb_attrs,
};

static const struct iio_info ads114s0xb_info = {
	.read_raw = ads114s0xb_read_raw,
	.attrs = &ads114s0xb_attr_group,
	.update_scan_mode = ads114s0xb_update_scan_mode,
};

// Simulated sensor raw values
static int sensor_raw_value_0 = 0xA510;
static irqreturn_t ads114s0xb_trigger_handler(int irq, void *private) {
	struct iio_poll_func *pf = private;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct ads114s0xb_private *ads114s0xb_priv = iio_priv(indio_dev);

	pr_info("ads114s0xb_trigger_handler: Called");
	
	ads114s0xb_priv->buffer[0] = ads114s0xb_read(indio_dev);
	// Mock
	ads114s0xb_priv->buffer[0] = sensor_raw_value_0;

	iio_push_to_buffers_with_timestamp(indio_dev, ads114s0xb_priv->buffer, 
		iio_get_time_ns(indio_dev));
	iio_trigger_notify_done(indio_dev->trig);
	return IRQ_HANDLED;
}

static const struct iio_buffer_setup_ops ads114s0xb_buffer_ops = {
	.preenable = ads114s0xb_buffer_preenable,
	.postdisable = ads114s0xb_buffer_postdisable,
};

/* ---- SPI Probe Function ---- */
static int ads114s0xb_probe(struct spi_device *spi)
{
	struct ads114s0xb_private *ads114s0xb_priv;
	struct iio_dev *indio_dev;
	int ret;
	const struct spi_device_id *spi_id = spi_get_device_id(spi);

	pr_info("ads114s0xb: Probing SPI driver\n");

	/* Allocate IIO device */
	indio_dev =
	    devm_iio_device_alloc(&spi->dev, 
		sizeof(struct ads114s0xb_private));
	if (!indio_dev)
		return -ENOMEM;

	ads114s0xb_priv = iio_priv(indio_dev);
	ads114s0xb_priv->spi = spi;
	ads114s0xb_priv->chip_info = 
		&ads114s0xb_chip_info_tbl[spi_id->driver_data];
	ads114s0xb_priv->reset_gpio =
	    devm_gpiod_get_optional(&spi->dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(ads114s0xb_priv->reset_gpio))
		dev_info(&spi->dev, "Reset GPIO not defined\n");

	mutex_init(&ads114s0xb_priv->lock);

	indio_dev->info = &ads114s0xb_info;
	indio_dev->name = spi_id->name;
	indio_dev->modes = INDIO_DIRECT_MODE | INDIO_BUFFER_TRIGGERED;

	indio_dev->channels = ads114s0xb_priv->chip_info->channels;
	indio_dev->num_channels = ads114s0xb_priv->chip_info->num_channels;

	ret = devm_iio_triggered_buffer_setup(&spi->dev, indio_dev, NULL,
		ads114s0xb_trigger_handler, NULL);
	if (ret) {
		dev_err(&spi->dev, "iio triggered buffer setup failed\n");
		return ret;
	}
	
	/* Register IIO device */
	ret = devm_iio_device_register(&spi->dev, indio_dev);
	if (ret) {
		pr_err("ads114s0xb: Failed to register IIO device\n");
		return ret;
	}

	spi_set_drvdata(spi, indio_dev);
	pr_info("ads114s0xb: SPI driver successfully registered\n");

	ads114s0xb_reset(indio_dev);

	return 0;
}

/* ---- SPI Device ID Table ---- */
static const struct spi_device_id spi_ads114s0xb_id[] = {
	{
		"ads114s06b",
		ADS114S06B_ID
	},
	{
		"ads114s08b",
		ADS114S08B_ID
	},
	{}
};

MODULE_DEVICE_TABLE(spi, spi_ads114s0xb_id);

static const struct of_device_id ads114s0xb_of_table[] = {
	{
		.compatible = "ti,ads114s06b"
	},
	{
		.compatible = "ti,ads114s08b"
	}, 
	{}
};
MODULE_DEVICE_TABLE(of, ads114s0xb_of_table);

/* ---- SPI Driver Structure ---- */
static struct spi_driver ads114s0xb_driver = {
	.driver =
	{
		.name = "ads114s0xb",
		.of_match_table = ads114s0xb_of_table,
	},
	.probe = ads114s0xb_probe,
	.id_table = spi_ads114s0xb_id,
};

/* ---- Register SPI Driver ---- */
module_spi_driver(ads114s0xb_driver);

MODULE_AUTHOR("Jairo Borba <gyrok42@gmail.com>");
MODULE_DESCRIPTION("TI ADS114s0xB multi-channel ADCs");
MODULE_LICENSE("GPL v2");
