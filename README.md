# Firmware of JianWei GasFlow Control System

## initialization

```shell
west init -m <git_url> <your_workspace>
cd <your_workspace>`
west update
```

## build
`west build -p -b rpi_pico -d build/app gasflow_firmware.git/app`

## check memory footprint

### RAM usage
`west build -d <build-directory> -t ram_report`

### ROM usage
`west build -d <build-directory> -t rom_report`
