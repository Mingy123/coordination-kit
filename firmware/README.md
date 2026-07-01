# DFR1154 Firmware — ESP-NOW Bridge + I2S Speaker

ESP-IDF v5.5.4 firmware for the DFRobot DFR1154 ESP32-S3 AI Camera board.
Runs an ESP-NOW bridge with an I2S speaker output (MAX98357A). USB
console via native USB-Serial-JTAG (CDC-ACM).

## Features

- **ESP-NOW bridge** — receives beacon packets from a C6 coordinator, sends ACKs
- **I2S speaker** — 440 Hz sine wave tone on MAX98357A amp (via `start_speaker()`)
- **SD card** — formatted on boot for logging

> Previous UVC webcam code is preserved at `main/usb_webcam.c` and
> `main/camera_pin.h` but excluded from the build. Re-enable by adding
> `usb_webcam.c` back to `main/CMakeLists.txt` and restoring camera
> dependencies in `idf_component.yml`.

## Requirements

- ESP-IDF v5.5.4 ([install guide](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s3/get-started/index.html))
- DFR1154 ESP32-S3 AI Camera board (OV3660 sensor + 16 MB flash)
- Optional: MAX98357A I2S speaker amp

## Build

```bash
cd firmware
source $HOME/esp/esp-idf/export.sh
SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.esp32s3" idf.py set-target esp32s3
idf.py build flash monitor
```

To target an ESP32-C6 instead:

```bash
SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.esp32c6" idf.py set-target esp32c6
```

## Flash

Plug the board into USB. Console appears as `/dev/ttyACM0` via USB-Serial-JTAG (CDC-ACM).

```bash
idf.py -p /dev/ttyACM0 flash monitor
```

## Wiring (DFR1154)

### Camera

Hard-wired on the DFR1154 PCB:

| Signal  | GPIO |
|---------|------|
| VSYNC   | 1    |
| HSYNC   | 2    |
| PCLK    | 15   |
| XCLK    | 5    |
| SDA     | 8    |
| SCL     | 9    |
| D0-Y9   | 4-7, 14, 16-18, 21 |

See `DFR1154_ESP32-S3_AI_Camera_Component_Reference.md` for full board docs.

### Speaker (MAX98357A)

| Signal | GPIO | Module pin |
|--------|------|------------|
| BCLK   | 45   | 51         |
| LRCLK  | 46   | 52         |
| DIN    | 42   | 48 (MTMS)  |
| SD#    | 40   | 46 (MTDO)  |
| GAIN   | 41   | 47 (MTDI)  |

## Notes

- S3 only. C6 lacks USB-OTG for future camera re-enable.
- PSRAM still enabled in defaults (general-purpose, not camera-specific).
