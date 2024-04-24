#pragma once

int acu20fd_init(const int iface, const uint8_t uid, bool digit_mode);
int acu20fd_set_pv(const int iface, const uint8_t uid, uint16_t *p_pv);
int acu20fd_get_pv(const int iface, const uint8_t uid, uint16_t *p_pv);
