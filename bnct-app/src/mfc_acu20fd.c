
#include "common.h"
#include <gasflow/acu20fd.h>

#include <zephyr/kernel.h>
#include <zephyr/modbus/modbus.h>

// modbus commuinication rx timeout (us)
#define MBC_RX_TIMEOUT 500000
#define ACU20FD_UINIT_ID 1

// modbus interface handle
static int mbc_iface;
static const uint8_t mbc_uid = ACU20FD_UINIT_ID;

/* poll pv */
static void mfc_acu20fd_poll() {
  uint16_t buf[2];
  if (!acu20fd_get_pv(mbc_iface, mbc_uid, buf)) {
    holding_reg[REG_MFC_RATE_PV]->value = buf[0];
    holding_reg[REG_MFC_RATE_PV + 1]->value = buf[1];
  }
  return;
}

POLL_HANDLER_DEFINE(acu20fd_poll, mfc_acu20fd_poll);

/* set pv */
static void mfc_acu20fd_holding_handler(uint16_t addr) {

  if ((addr == REG_MFC_RATE_SV || addr == (REG_MFC_RATE_SV+1)) && 
      (coil_reg[COIL_ONOFF_MFC]->value)) {
    uint16_t buf[2] = {holding_reg[REG_MFC_RATE_SV]->value,
                       holding_reg[REG_MFC_RATE_SV + 1]->value};

    coil_reg[COIL_STATUS_LATEST]->value =
        (acu20fd_set_pv(mbc_iface, mbc_uid, buf) < 0) ? false : true;
  }

  return;
}

HOLDING_REG_HANDLER_DEFINE(acu20fd_holding, mfc_acu20fd_holding_handler);

/* turn on/off mfc */
static void mfc_acu20fd_coil_handler(uint16_t addr) {

  switch (addr) {
  case COIL_ONOFF_MFC:
    uint16_t buf[2] = {0, 0};
    if (coil_reg[COIL_ONOFF_MFC]->value) {
        buf[0] = holding_reg[REG_MFC_RATE_SV]->value;
        buf[1] = holding_reg[REG_MFC_RATE_SV + 1]->value;
    };
    coil_reg[COIL_STATUS_LATEST]->value =
        (acu20fd_set_pv(mbc_iface, mbc_uid, buf) < 0) ? false : true;


    break;
  case COIL_DEFAULT_RATE_MFC:
    holding_reg[REG_MFC_RATE_SV]->value =
        holding_reg[REG_MFC_RATE_DEFAULT]->value;
    holding_reg[REG_MFC_RATE_SV + 1]->value =
        holding_reg[REG_MFC_RATE_DEFAULT + 1]->value;
    break;
  default:
    break;
  }

  return;
}

COIL_REG_HANDLER_DEFINE(acu20fd_coil, mfc_acu20fd_coil_handler);

/* modus interface to acu20fd (a client) */
const static struct modbus_iface_param mbc_param = {
    .mode = MODBUS_MODE_RTU,
    .rx_timeout = MBC_RX_TIMEOUT,
    .serial = {.baud = 115200,
               .parity = UART_CFG_PARITY_NONE,
               .stop_bits_client = UART_CFG_STOP_BITS_1}};

#define MODBUS_CLIENT_NODE DT_ALIAS(modbus_client)

static int mbc_init(void) {
  char const iface_name[] = {DEVICE_DT_NAME(MODBUS_CLIENT_NODE)};

  mbc_iface = modbus_iface_get_by_name(iface_name);
  if (mbc_iface < 0) {
    printk("Failed to get iface index for %s\n", iface_name);
    return mbc_iface;
  }

  return modbus_init_client(mbc_iface, mbc_param);
}

//
static int mfc_acu20fd_init(void) {

  coil_reg[COIL_STATUS_MFC]->value = false;

  int err = mbc_init();
  if (err < 0) {
    printk("Modbus RTU client initialization failed\n");
    return err;
  }

  // needed for mfc power-up
  k_msleep(1000);

  err = acu20fd_init(mbc_iface, mbc_uid, true);
  if (err < 0) {
    printk("Setting ACU20FD digit mode failed\n");
    return err;
  }
  coil_reg[COIL_STATUS_MFC]->value = true;

  // [todo] better method?
  holding_reg[REG_MFC_RATE_SV]->value = 0;
  holding_reg[REG_MFC_RATE_SV + 1]->value = 16928;
  holding_reg[REG_MFC_RATE_DEFAULT]->value = 0;
  holding_reg[REG_MFC_RATE_DEFAULT + 1]->value = 16928;

  return 0;
}

SYS_INIT(mfc_acu20fd_init, POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY);
