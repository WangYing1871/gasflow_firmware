/*
 * Support MFC acu20fd:
 * - use modbus interface handle (int)
 * - convert flow rate to 16-bit decimal and 16-bit fractional part (after
 * x1000000)
 *
 */

#include <zephyr/modbus/modbus.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel.h>

#define ACU20FD_REG_READ_PV 0x0010
#define ACU20FD_REG_READ_CV 0x001C
#define ACU20FD_REG_RESET_CV 0x001C
#define ACU20FD_REG_SET_MODE 0x0074
#define ACU20FD_REG_SET_PV 0x006A
#define ACU20FD_REG_SET_ZERO 0x0076
#define ACU20FD_REG_SET_COEFF 0x0072

static uint16_t __attribute__((unused)) acu20fd_reset_cv[] = {0x0000, 0x0000};
static uint16_t acu20fd_digit_mode[] = {0x0000, 0x41D0};
static uint16_t __attribute__((unused)) acu20fd_analog_mode[] = {0x0000, 0x41C8};
static uint16_t __attribute__((unused)) acu20fd_set_zero[] = {0x0000, 0x0000};
static uint16_t __attribute__((unused)) acu20fd_cancel_zero[] = {0x3F80, 0x0000};

#define ACU20FD_DEAD_TIME_MS 200

/* set work mode: analog (digit_mode: false), digi (digit_mode: true) */
int acu20fd_init(const int iface, const uint8_t uid, bool digit_mode) {

  if (!digit_mode) return 0;

  int err = modbus_write_holding_regs(iface, uid, ACU20FD_REG_SET_MODE,
                                      acu20fd_digit_mode,
                                      ARRAY_SIZE(acu20fd_digit_mode));
  if (err != 0) {
    printk("set accu20fd to digit-mode failed\n");
    return err;
  }

  k_msleep(ACU20FD_DEAD_TIME_MS);
  return 0;
}

/* set & read pv */
int acu20fd_set_pv(const int iface, const uint8_t uid, uint16_t *p_pv) {
  int err = modbus_write_holding_regs(iface, uid, ACU20FD_REG_SET_PV, p_pv, 2);
  if (err != 0) {
    printk("set accu20fd PV failed\n");
    return err;
  }

  k_msleep(ACU20FD_DEAD_TIME_MS);
  return 0;
}

int acu20fd_get_pv(const int iface, const uint8_t uid, uint16_t *p_pv) 
{
  int err = modbus_read_holding_regs(iface, uid, ACU20FD_REG_READ_PV,
                                     p_pv, 2);
  if (err != 0) {
    printk("get accu20fd PV failed\n");
    return err;
  }

  return 0;
}

