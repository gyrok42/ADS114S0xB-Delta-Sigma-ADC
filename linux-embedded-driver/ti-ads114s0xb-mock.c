/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#define ADS114S0XB_MAX_CHANNELS 12 /* Number of 8-bit channels */
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

struct ads114s0xb_private {
  u8 channels[ADS114S0XB_MAX_CHANNELS]; /* Internal register storage */
  u8 attributes[ADS114S0XB_NUM_ATTRIBUTES];
  struct mutex lock;
};

static struct iio_dev *ads114s0xb_indio_dev;

static int ads114s0xb_read_raw(struct iio_dev *indio_dev,
                               struct iio_chan_spec const *chan, int *val,
                               int *val2, long mask) {
  struct ads114s0xb_private *ads114s0xb_priv = iio_priv(indio_dev);

  if (chan->address >= ADS114S0XB_MAX_CHANNELS)
    return -EINVAL;

  mutex_lock(&ads114s0xb_priv->lock);
  *val = ads114s0xb_priv->channels[chan->address];
  mutex_unlock(&ads114s0xb_priv->lock);

  pr_info("ads114s0xb: Read register[%lu] = %d\n", chan->address, *val);
  return IIO_VAL_INT;
}

static ssize_t ads114s0xb_attr_get(struct device *dev, struct device_attribute *attr,
                        char *buf) {
  // struct iio_dev *indio_dev = dev_to_iio_dev(dev);
  // struct ads114s0xb_private *ads114s0xb_priv = iio_priv(indio_dev);
  struct iio_dev_attr *iio_attr = to_iio_dev_attr(attr);
  return sprintf(buf, "%02X\n",
                 (u8)(iio_attr->address)); // ads114s0xb_priv->attributes[0]);
}

static ssize_t ads114s0xb_attr_set(struct device *dev, struct device_attribute *attr,
                        const char *buf, size_t count) {
  struct iio_dev *indio_dev = dev_to_iio_dev(dev);
  struct ads114s0xb_private *ads114s0xb_priv = iio_priv(indio_dev);
  int val;

  if (kstrtoint(buf, 10, &val) < 0)
    return -EINVAL;

  mutex_lock(&ads114s0xb_priv->lock);
  ads114s0xb_priv->attributes[0] = (u8)val;
  mutex_unlock(&ads114s0xb_priv->lock);

  pr_info("ads114s0xb: Attribute %s set to %d\n", attr->attr.name, val);
  return count;
}

#define IIO_RW_ATTRIBUTE(name, addr)                                           \
  static IIO_DEVICE_ATTR(name, 0644, ads114s0xb_attr_get, ads114s0xb_attr_set, addr);

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

static struct attribute *ads114s0xb_attrs[] = {
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

/* Define 18 channels (each represents a register) */
static struct iio_chan_spec ads114s0xb_channels[ADS114S0XB_MAX_CHANNELS];

static const struct iio_info ads114s0xb_info = {
    .read_raw = ads114s0xb_read_raw,
    .attrs = &ads114s0xb_attr_group,
};

static int __init ads114s0xb_init(void) {
  struct ads114s0xb_private *ads114s0xb_priv;
  int i, ret;

  pr_info("ads114s0xb: Initializing standalone IIO driver\n");

  /* Allocate IIO device */
  ads114s0xb_indio_dev =
      iio_device_alloc(NULL, sizeof(struct ads114s0xb_private));
  if (!ads114s0xb_indio_dev)
    return -ENOMEM;

  ads114s0xb_priv = iio_priv(ads114s0xb_indio_dev);
  mutex_init(&ads114s0xb_priv->lock);

  ads114s0xb_indio_dev->info = &ads114s0xb_info;
  ads114s0xb_indio_dev->name = "ads114s0xb";
  ads114s0xb_indio_dev->modes = INDIO_DIRECT_MODE;

  /* Initialize channels */
  for (i = 0; i < ADS114S0XB_MAX_CHANNELS; i++) {
    ads114s0xb_channels[i].type = IIO_VOLTAGE;
    ads114s0xb_channels[i].indexed = 1;
    ads114s0xb_channels[i].channel = i;
    ads114s0xb_channels[i].address = i;
    ads114s0xb_channels[i].info_mask_separate = BIT(IIO_CHAN_INFO_RAW);
  }

  ads114s0xb_indio_dev->channels = ads114s0xb_channels;
  ads114s0xb_indio_dev->num_channels = ADS114S0XB_MAX_CHANNELS;

  /* Register IIO device */
  ret = iio_device_register(ads114s0xb_indio_dev);
  if (ret) {
    pr_err("ads114s0xb: Failed to register IIO device\n");
    iio_device_free(ads114s0xb_indio_dev);
    return ret;
  }

  pr_info("ads114s0xb: Standalone IIO driver successfully registered\n");
  return 0;
}

static void __exit ads114s0xb_exit(void) {
  pr_info("ads114s0xb: Removing standalone IIO driver\n");

  /* Unregister and free IIO device */
  iio_device_unregister(ads114s0xb_indio_dev);
  iio_device_free(ads114s0xb_indio_dev);
}

module_init(ads114s0xb_init);
module_exit(ads114s0xb_exit);

MODULE_AUTHOR("Jairo Borba <gyrok42@gmail.com>");
MODULE_DESCRIPTION("Minimal IIO driver with user-space communication");
MODULE_LICENSE("GPL v2");