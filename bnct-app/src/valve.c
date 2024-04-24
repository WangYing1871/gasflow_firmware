
#include "common.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#if !DT_NODE_EXISTS(DT_NODELABEL(valve_switch))
#error "compiler: dts node with lablel valve_switch not defined"
#else
#define VALVE_NODE DT_NODELABEL(valve_switch)
#endif

static const struct gpio_dt_spec valve = GPIO_DT_SPEC_GET(VALVE_NODE, gpios);

static void valve_coil_register_handler(uint8_t addr) {
  if (addr == COIL_ONOFF_VALVE) {
    gpio_pin_configure_dt(&valve, coil_reg[COIL_ONOFF_VALVE]->value
                                      ? GPIO_OUTPUT_ACTIVE
                                      : GPIO_OUTPUT_INACTIVE);
  }

  return;
}

COIL_REG_HANDLER_DEFINE(valve_coil_reg, valve_coil_register_handler);

static int valve_init(void) {
  coil_reg[COIL_STATUS_VALVE]->value = false;

  if (!gpio_is_ready_dt(&valve)) {
    printk("The valve GPIO port is not ready.\n");
    return -ENOENT;
  }
  coil_reg[COIL_STATUS_VALVE]->value = true;

  return 0;
}

SYS_INIT(valve_init, POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY);
