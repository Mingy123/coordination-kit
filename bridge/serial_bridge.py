"""Reads serial button events from the S3 bridge and publishes them as an
LSL string outlet.

With a calibrated button_map.json (see setup.py), mapped buttons are
normalized: press -> "<label>", release -> "<label>_up", regardless of
whether the physical switch is active-low or active-high. Without a map,
raw lines are forwarded as before.
"""
import argparse
import json
import sys
from pathlib import Path

import serial
from pylsl import StreamInfo, StreamOutlet

MAP_PATH = Path(__file__).parent / "button_map.json"
STREAM_NAME = "CoordinationKit_Events"


def load_map(path: Path = MAP_PATH) -> dict:
    if not path.exists():
        print(f"bridge: no map at {path}, raw passthrough mode", file=sys.stderr)
        return {}
    data = json.loads(path.read_text())
    return {b["mac"]: b for b in data["buttons"]}


def main(port: str = "/dev/ttyUSB0", baud: int = 115200):
    mapping = load_map()
    if mapping:
        print(f"bridge: loaded {len(mapping)} buttons from {MAP_PATH}", file=sys.stderr)

    info = StreamInfo(STREAM_NAME, "Markers", 1, 0, "string", "coordbridge01")
    outlet = StreamOutlet(info)

    with serial.Serial(port, baud, timeout=1) as ser:
        print(f"bridge: connected to {port} at {baud}", file=sys.stderr)
        while True:
            line = ser.readline().decode("utf-8", errors="replace").strip()
            if not line:
                continue
            parts = line.split()
            marker = line
            if len(parts) == 3 and parts[0] == "btn":
                level, mac = parts[1], parts[2]
                btn = mapping.get(mac)
                if btn:
                    pressed = (level == str(btn["pressed"]))
                    marker = btn["label"] if pressed else f"{btn['label']}_up"
            outlet.push_sample([marker])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", default="/dev/ttyUSB0")
    parser.add_argument("--baud", type=int, default=115200)
    args = parser.parse_args()
    main(args.port, args.baud)
