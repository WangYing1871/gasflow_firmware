cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(bnct_firmware)

target_sources(app PRIVATE
  src/mode.c
  src/modbus.c
  src/heartbeat.c
  src/status.c
  src/registers.c
  src/valve.c
  src/pump.c
)

target_sources_ifdef(CONFIG_BME280 app PRIVATE src/bmp280.c)
