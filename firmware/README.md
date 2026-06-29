# USB Webcam — DFR1154 (ESP32-S3 + OV3660)

UVC device firmware for the DFRobot DFR1154 ESP32-S3 AI Camera board.
Presents as a standard USB Video Class camera when plugged into a host.

## Requirements

- ESP-IDF v5.4 ([install guide](https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32s3/get-started/index.html))
- ESP32-S3 board with camera + USB (DFR1154 or custom wiring)

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

## Flash

Plug the board into USB. The bootloader hook disables the USB-Serial-JTAG
interface so the UVC device appears on the host.

```bash
idf.py -p /dev/ttyACM0 flash monitor
```

On the host, verify with `lsusb` — should show "Espressif ESP UVC Device".

## Wiring (DFR1154)

Camera is hard-wired on the DFR1154 PCB. Pin mapping:

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

## Configuration

```bash
idf.py menuconfig
```

- **USB WebCam config** → XCLK frequency (default 20MHz)
- **USB Device UVC** → resolution, frame rate, isochronous/bulk mode

## Notes

- S3 only. C6 lacks USB-OTG peripheral mode.
- MJPEG only (sensor JPEG compression required).
- PSRAM must be present for frame buffers.
