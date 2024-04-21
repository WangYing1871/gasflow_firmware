#pragma once


/*** holding registers ***/

/// status registers

#define REG_PV_MFC 0
#define REG_PV_PUMPSPEED 1
#define REG_PV_PRESSURE 2
#define REG_PV_TEMPERATURE 3

// voltage present value (V)
#define REG_SV_MFC 4
#define REG_SV_PUMPSPEED 5

// status polling interval (in ms)
#define REG_UPDATE_INTERVAL 6

// system alive time (high-16 bits & low-16 bits)
/// alive-time = polling-times * update_interval
#define REG_HEARTBEAT_LW 7
#define REG_HEARTBEAT_HI 8

// others
#define REG_TEMPERATURE_INT 9
#define REG_TEMPERATURE_FLT 10
#define REG_PRESSURE_INT 11
#define REG_PRESSURE_FLT 12

// working modes, valid values:
// - 1 -> flow-mode,
// - 2 -> recycle-mode
// - 3 -> mix-mode (auto switching between flow & recycle)
#define REG_SET_MODE 13
#define REG_CUR_MODE 14
#define REG_CUR_STATE 15

// duration of flow & recycle state in mix-mode (unit:s)
#define REG_FLOW_DURATION 16
#define REG_RECYCLE_DURATION 17

// pump speed in recycle-state (unit: permil of full-speed)
// default speed is read-only
#define REG_PUMP_SPEED 18
#define REG_PUMP_SPEED_DEFAULT 19
#define REG_PUMP_SPEED_MIN 20
#define REG_PUMP_SPEED_MAX 21

#define REG_END 22

/*** coils ***/

/// control command coils
#define COIL_ONOFF_MFC 0
#define COIL_ONOFF_PUMP 1
#define COIL_ONOFF_VALVE 2

#define COIL_DEFAULT_SPEED_PUMP 3
#define COIL_DEFAULT_SPEED_MFC 4

/* #define COIL_AUTOMATIC 4 */

#define COIL_END 5
