/*
 * Infinite loop of switching between 3 working modes in response to latest
 * mode-switching:
 *   - flow
 *   - recycle
 *   - mixed : automatic switching between flow and recycle
 *
 * The power-on default state is: flow-mode
 *
 */

#include "common.h"

#include <zephyr/kernel.h>

#define DEFAULT_FLOW_DURATION 300
#define DEFAULT_RECYCLE_DURATION 1200

/* mode-switching signal facility */
static struct k_poll_signal mode_sig = K_POLL_SIGNAL_INITIALIZER(mode_sig);
static struct k_poll_event mode_event = K_POLL_EVENT_INITIALIZER(
    K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY, &mode_sig);

static void mode_holding_register_handler(uint16_t addr) {
  if (addr != REG_SET_MODE)
    return;

  // lock & raise signal
  k_poll_signal_raise(mode_event.signal, 0);
}

HOLDING_REG_HANDLER_DEFINE(mode_holding_reg, mode_holding_register_handler);

/* working mode setup functions */
enum working_mode {
  DEFAULT_MODE = 0, // not used yet
  FLOW_MODE,        // current power-on default
  RECYCLE_MODE,
  MIX_MODE
};

enum working_state {
  INVALID_STATE = 0,
  FLOW_STATE,
  RECYCLE_STATE
};

inline static void lock_state()
{
  k_mutex_lock(&coil_reg[COIL_ONOFF_PUMP]->mutex, K_FOREVER);
  k_mutex_lock(&coil_reg[COIL_ONOFF_MFC]->mutex, K_FOREVER);
  k_mutex_lock(&coil_reg[COIL_ONOFF_VALVE]->mutex, K_FOREVER);

  k_mutex_lock(&holding_reg[REG_CUR_STATE]->mutex, K_FOREVER);
}

inline static void unlock_state() {
  k_mutex_unlock(&coil_reg[COIL_ONOFF_PUMP]->mutex);
  k_mutex_unlock(&coil_reg[COIL_ONOFF_MFC]->mutex);
  k_mutex_unlock(&coil_reg[COIL_ONOFF_VALVE]->mutex);

  k_mutex_unlock(&holding_reg[REG_CUR_STATE]->mutex);
}

inline static void setup_flow_state() {
  lock_state();
  
  // 1. stop pump
  coil_reg[COIL_ONOFF_PUMP]->value = false;
  COIL_REG_HANDLER_FOREACH(handler) { handler->callback(COIL_ONOFF_PUMP); }

  // 2. turn on MFC
  coil_reg[COIL_ONOFF_MFC]->value = true;
  COIL_REG_HANDLER_FOREACH(handler) { handler->callback(COIL_ONOFF_MFC); }

  // 3. open valve
  coil_reg[COIL_ONOFF_VALVE]->value = true;
  COIL_REG_HANDLER_FOREACH(handler) { handler->callback(COIL_ONOFF_VALVE); }

  // 4. update current state
  holding_reg[REG_CUR_STATE]->value = FLOW_STATE;

  unlock_state();
}

inline static void setup_recycle_state() {
  lock_state();
  
  // 1. close valve
  coil_reg[COIL_ONOFF_VALVE]->value = false;
  COIL_REG_HANDLER_FOREACH(handler) { handler->callback(COIL_ONOFF_VALVE); }

  // 2. turn off MFC
  coil_reg[COIL_ONOFF_MFC]->value = false;
  COIL_REG_HANDLER_FOREACH(handler) { handler->callback(COIL_ONOFF_MFC); }

  // 3. start pump
  coil_reg[COIL_ONOFF_PUMP]->value = true;
  COIL_REG_HANDLER_FOREACH(handler) { handler->callback(COIL_ONOFF_PUMP); }

  // 4. update current state
  holding_reg[REG_CUR_STATE]->value = RECYCLE_STATE;

  unlock_state();
}

inline static void setup_mode(enum working_mode mode) {
  k_mutex_lock(&holding_reg[REG_CUR_MODE]->mutex, K_FOREVER);
  holding_reg[REG_CUR_MODE]->value = mode;
  k_mutex_unlock(&holding_reg[REG_CUR_MODE]->mutex);
}

/* mix-mode timer */
static struct k_timer to_flow_mode_timer;
static struct k_timer to_recycle_mode_timer;

static void to_flow_mode_timer_callback(struct k_timer *timer) {
  setup_flow_state();
  k_timer_start(&to_recycle_mode_timer, // one-shot timer
                K_SECONDS(holding_reg[REG_FLOW_DURATION]->value), K_NO_WAIT);
  return;
}

static void to_recycle_mode_timer_callback(struct k_timer *timer) {
  setup_recycle_state();
  k_timer_start(&to_flow_mode_timer, // one-shot timer
                K_SECONDS(holding_reg[REG_RECYCLE_DURATION]->value), K_NO_WAIT);
  return;
}

inline static void setup_mix_mode() {
  if (holding_reg[REG_CUR_STATE]->value == FLOW_STATE) {
    to_recycle_mode_timer_callback(NULL);
  } else {
    to_flow_mode_timer_callback(NULL);
  }

  return;
}

/* main thread: polling the current working mode status in an infinite loop */

int main(void) {

  /* power-on mode init: flow-mode */
  holding_reg[REG_SET_MODE]->value = FLOW_MODE;
  setup_flow_state();
  setup_mode(FLOW_MODE);

  /* init timer for mix-mode's auto-switching */
  holding_reg[REG_FLOW_DURATION]->value = DEFAULT_FLOW_DURATION;
  holding_reg[REG_RECYCLE_DURATION]->value = DEFAULT_RECYCLE_DURATION;

  k_timer_init(&to_flow_mode_timer, to_flow_mode_timer_callback, NULL);
  k_timer_init(&to_recycle_mode_timer, to_recycle_mode_timer_callback, NULL);

  /* start polling mode change */
  while (1) {

    // receiving cmd setting new mode
    k_poll(&mode_event, 1, K_FOREVER);

    unsigned int signaled;
    int result;
    k_poll_signal_check(mode_event.signal, &signaled, &result);

    if (signaled == 0)
      continue;

    // switch to new working mode
    if (holding_reg[REG_CUR_MODE]->value == MIX_MODE) {
      k_timer_stop(&to_flow_mode_timer);
      k_timer_stop(&to_recycle_mode_timer);
    }

    switch (holding_reg[REG_SET_MODE]->value) {
    case FLOW_MODE:
      setup_flow_state();
      setup_mode(FLOW_MODE);
      break;
    case RECYCLE_MODE:
      setup_recycle_state();
      setup_mode(RECYCLE_MODE);
      break;
    case MIX_MODE:
      setup_mix_mode();
      setup_mode(MIX_MODE);
      break;
    default:
      break;
    }

    // ready for next mode-switching
    k_poll_signal_reset(mode_event.signal);
    mode_event.state = K_POLL_STATE_NOT_READY;
  }

  return 0;
}
