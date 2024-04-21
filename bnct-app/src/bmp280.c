#include "common.h"

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

#define BMP280_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(bosch_bme280)
static const struct device *dev;

/* status polling handler */
static void bmp280_poll() {
  struct sensor_value temp, pressure;
  sensor_sample_fetch(dev);

  // [todo] value interpretation to be confirmed
  sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
  holding_reg[REG_TEMPERATURE_INT]->value = temp.val1 & BIT_MASK(16);
  holding_reg[REG_TEMPERATURE_FLT]->value = temp.val2 & BIT_MASK(16); // = flt/1000000

  sensor_channel_get(dev, SENSOR_CHAN_PRESS, &pressure);
  holding_reg[REG_PRESSURE_INT]->value = pressure.val1 & BIT_MASK(16);
  holding_reg[REG_PRESSURE_FLT]->value = pressure.val2 & BIT_MASK(16); // = flt/1000000

  return;
}

POLL_HANDLER_DEFINE(bmp280_poll, bmp280_poll);

//
static int bmp280_init(void) {
  dev = DEVICE_DT_GET(BMP280_NODE);

  if (!device_is_ready(dev)) {
    printk("\nError: Device \"%s\" is not ready; "
           "check the driver initialization logs for errors.\n",
           dev->name);
    return -ENOENT;
  }

  return 0;
}

SYS_INIT(bmp280_init, POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY);
