#pragma once

#define MFC_REG_READ_PV 0x0010
#define MFC_REG_READ_CV 0x001C
#define MFC_REG_RESET_CV 0x001C
#define MFC_REG_SET_MODE 0x0074
#define MFC_REG_SET_PV 0x006A
#define MFC_REG_SET_ZERO 0x0076
#define MFC_REG_SET_COEFF 0x0072

#define MFC_CMD_SIZE 0x0002
static uint16_t mfc_cmd_reset_cv[MFC_CMD_SIZE] = {0x0000, 0x0000};
static uint16_t mfc_cmd_digit_mode[MFC_CMD_SIZE] = {0x0000, 0x41D0};
static uint16_t mfc_cmd_analog_mode[MFC_CMD_SIZE] = {0x0000, 0x41C8};
static uint16_t mfc_cmd_set_zero[MFC_CMD_SIZE] = {0x0000, 0x0000};
static uint16_t mfc_cmd_cancel_zero[MFC_CMD_SIZE] = {0x3F80, 0x0000};
