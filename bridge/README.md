# bridge — Serial-to-LSL Outlet

Reads newline-delimited events from an ESP32 over UART and publishes them as an [LSL](https://labstreaminglayer.org/) string stream.

## Stream

- **Name:** `CoordinationKit_Events`
- **Type:** `Markers`
- **Format:** Single string sample per event

## Usage

```bash
pip install -r requirements.txt
python serial_bridge.py --port /dev/ttyUSB0 --baud 115200
```

All arguments are optional — defaults to `/dev/ttyUSB0` at 115200 baud.

## Files

- `serial_bridge.py` — main script with argparse
- `requirements.txt` — `pylsl`, `pyserial`
