#include "common.h"

#include <zephyr/devicetree/pwms.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>

#if !DT_NODE_EXISTS(DT_NODELABEL(pump))
#error "compiler: dts node with lablel pump not defined"
#else
#define PUMP_NODE DT_NODELABEL(pump)
#endif

static const struct pwm_dt_spec pump = PWM_DT_SPEC_GET(PUMP_NODE);
static const uint32_t period = DT_PWMS_PERIOD(PUMP_NODE);
static const uint32_t step = period / 1000; // per mille
static const uint32_t min_speed = DT_PROP(PUMP_NODE, min_pulse) / step; //  < min_pulse = turn-off pump
static const uint32_t max_speed = DT_PROP(PUMP_NODE, max_pulse) / step;
static const uint32_t default_speed = DT_PROP(PUMP_NODE, default_speed); // unit: permil, i.e. 1/1000

// [todo] poll handler (present speed: REG_PUMP_SPEED_PV)

// coil handler
static void pump_coil_register_handler(uint8_t addr) {

  switch (addr) {
  case COIL_ONOFF_PUMP:
    pwm_set_pulse_dt(&pump, coil_reg[COIL_ONOFF_PUMP]->value
                                ? holding_reg[REG_PUMP_SPEED_SV]->value * step
                                : 0);
    break;
  case COIL_DEFAULT_SPEED_PUMP:
    holding_reg[REG_PUMP_SPEED_SV]->value = default_speed;
    break;
  default:
    break;
  }

  return;
}

COIL_REG_HANDLER_DEFINE(pump_coil_reg, pump_coil_register_handler);

// holding register handler
static void pump_holding_register_handler(uint8_t addr) {
  if (addr == REG_PUMP_SPEED_SV && coil_reg[COIL_ONOFF_PUMP]->value) {
      pwm_set_pulse_dt(&pump, holding_reg[REG_PUMP_SPEED_SV]->value * step);
  }
  return;
}

HOLDING_REG_HANDLER_DEFINE(pump_holding_reg, pump_holding_register_handler);

// init
static int pump_init(void) {
  if (!pwm_is_ready_dt(&pump)) {
    printk("The pump is not ready.\n");
    return -ENOENT;
  }

  holding_reg[REG_PUMP_SPEED_SV]->value = default_speed;
  holding_reg[REG_PUMP_SPEED_DEFAULT]->value = default_speed;
  holding_reg[REG_PUMP_SPEED_MIN]->value = min_speed;
  holding_reg[REG_PUMP_SPEED_MAX]->value = max_speed;

  return 0;
}

SYS_INIT(pump_init, POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY);
