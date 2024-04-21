#pragma once

#include "macro_internal.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/iterable_sections.h>

struct holding_register_t {
  uint16_t value;
  struct k_mutex mutex;
};

struct coil_register_t {
  bool value;
  struct k_mutex mutex;
};

#define HOLDING_REG_DEFINE(addr, value)                                        \
  XLAB_REG_DEFINE(holding_register_t, holding, addr, value)
#define COIL_REG_DEFINE(addr, value)                                           \
  XLAB_REG_DEFINE(coil_register_t, coil, addr, value)
