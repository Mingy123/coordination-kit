# DFR1154 Firmware — UVC Webcam + I2S Speaker

ESP-IDF v5.4 firmware for the DFRobot DFR1154 ESP32-S3 AI Camera board.
Presents as a standard USB Video Class (UVC) webcam with an optional I2S
speaker output (MAX98357A).

## Features

- **UVC webcam** — MJPEG video at 720p (default) over USB
- **I2S speaker** — 440 Hz sine wave tone on MAX98357A amp (via `start_speaker()`)
- **Bootloader hook** — disables USB-Serial-JTAG D+ pullup so the UVC device
  claims the USB data lines

## Requirements

- ESP-IDF v5.4 ([install guide](https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32s3/get-started/index.html))
- DFR1154 ESP32-S3 AI Camera board (OV3660 sensor + 16 MB flash)
- Optional: MAX98357A I2S speaker amp

## Build

```bash
cd firmware
source $HOME/esp/esp-idf/export.sh
SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.esp32s3" idf.py set-target esp32s3
idf.py build flash monitor
```

All dependencies (`esp32-camera`, `usb_device_uvc`, `tinyusb`, `cmake_utilities`)
are fetched by the IDF component manager during the first build — no manual
cloning.

To target an ESP32-C6 instead:

```bash
SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.esp32c6" idf.py set-target esp32c6
```

(Note: C6 lacks USB-OTG peripheral mode, so UVC is S3-only for now.)

## Flash

Plug the board into USB. The bootloader hook disables the USB-Serial-JTAG
interface so the UVC device appears on the host.

```bash
idf.py -p /dev/ttyACM0 flash monitor
```

On the host, verify with `lsusb` — should show "Espressif ESP UVC Device".

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

## Configuration

```bash
idf.py menuconfig
```

- **USB WebCam config** → XCLK frequency (default 20 MHz)
- **USB Device UVC** → resolution, frame rate, isochronous/bulk mode

## Notes

- S3 only for UVC. C6 lacks USB-OTG peripheral mode.
- MJPEG only (sensor JPEG compression required).
- PSRAM must be present for camera frame buffers.
- Console UART runs at 2 Mbaud to keep USB free for UVC.
