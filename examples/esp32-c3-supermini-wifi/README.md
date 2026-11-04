# ESP32-C3 SuperMini Wi-Fi example

Build and upload from this directory:

```sh
pio run -e esp32-c3-supermini -t upload --upload-port /dev/ttyACM0
pio device monitor -p /dev/ttyACM0 -b 115200
```

Adjust the port and pins for your board. The example uses GPIO8 for the active-low LED, GPIO9 for the BOOT button and GPIO1 for ADC input. GPIO9 is a boot-strapping pin; keep external circuitry from interfering with boot.

Connect the editor over USB serial and open Device > Network to see the connection status and device IP. Edit up to three networks in priority order, or use Scan networks to scan from the device. Drag a saved network or use the arrow buttons to reorder it; use Forget to remove it and its password. A blank password keeps the existing password unless Replace password is checked; changing the network name always replaces it. The device tries each configured network in order and continues running the PLC while disconnected. After power cycling, it reloads the networks from ESP32 NVS.

The runtime enables this ESP32-specific feature with `PLCRUNTIME_WIFI_MANAGER`. Passwords are never returned by the WiFi management commands and are not saved in the editor project. They are stored in ESP32 NVS and sent over serial (or the editor's server-backed serial connection) without encryption; use ESP32 flash/NVS encryption and a trusted connection if physical or network access is a concern. Anyone with serial access can change or erase the configured networks.

PLC programming and monitoring still use USB serial: the runtime's current TCP transport does not pass received commands to its serial command parser. WiFi connectivity alone does not enable remote programming.

Program storage uses ESP32 EEPROM emulation in the custom 128 KiB NVS partition. This partition layout is intended for a 4 MiB device and does not provide OTA application slots. Changing partition layouts can invalidate existing stored data; back up anything important first. Verify persistence by downloading a PLC program, power-cycling the board, and checking that it reloads.

The example includes the runtime's core header directly to avoid an unrelated linker error in the current runtime test header. It uses the surrounding runtime through a PlatformIO symlink.