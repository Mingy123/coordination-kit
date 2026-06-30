# coordination-kit

Button / foot-switch → ESP32 → serial → LSL pipeline for real-time neuroscience coordination tasks.

## Architecture

```
┌─────────────┐     ┌─────────────────┐     ┌───────────────┐
│  FTDI hand   │     │                 │     │               │
│  buttons     │────▶│  ESP32-S3       │────▶│  bridge/      │────▶ LSL
│  Keyboard    │     │  (UVC webcam +  │     │  serial_bridge│      outlet
│  foot switch │     │   I2S speaker)  │     │               │
└─────────────┘     └─────────────────┘     └───────────────┘
       ▲                     ▲
    poc/                  firmware/
  (laptop POC)         (ESP-IDF C)
```

## Components

| Directory | Status | Description |
|-----------|--------|-------------|
| `poc/` | ✅ Working | Laptop proof-of-concept — Pygame UI reading two FTDI hand buttons + keyboard foot switches (2=left, 4=right) |
| `firmware/` | ✅ Active | ESP-IDF v5.4 firmware for the DFR1154 ESP32-S3 AI Camera board. USB Video Class (UVC) webcam + I2S audio output (MAX98357A) |
| `bridge/` | ✅ Working | Python serial-to-LSL script — reads events from the ESP32 over UART and pushes them as an LSL string outlet |

## Quick start

```bash
# POC — laptop buttons + foot switches
cd poc
pip install -r requirements.txt
python ui.py

# Firmware — requires ESP-IDF v5.4
cd firmware
source $HOME/esp/esp-idf/export.sh
SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.esp32s3" idf.py set-target esp32s3
idf.py build flash monitor

# Bridge — serial-to-LSL
cd bridge
pip install -r requirements.txt
python serial_bridge.py --port /dev/ttyUSB0
```

## License

MIT
