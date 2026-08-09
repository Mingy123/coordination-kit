#!/usr/bin/env python3
"""Standalone button-mapping setup/test for the coordination kit.

Reads raw `btn <level> <mac>` events from the S3 bridge over serial,
prompts the user to press each button once (press + release), learns the
MAC and pressed level for each, and saves a MAC -> label map to
button_map.json. After calibrating, keeps printing mapped events so you
can verify the setup live. No LSL dependency.

Usage:
  python3 setup.py                        # calibrate + live-print events
  python3 setup.py --buttons "left_hand,right_foot"
  python3 setup.py --monitor              # just watch raw events
  python3 setup.py --selftest             # run logic self-check
"""
import argparse
import json
import sys
import time
from pathlib import Path

import serial

MAP_PATH = Path(__file__).parent / "button_map.json"
DEFAULT_BUTTONS = ["left_hand", "right_hand", "left_foot", "right_foot"]
PROMPT_TIMEOUT_S = 20.0


def parse_event(line: str):
    """'btn 0 aa:bb:cc:dd:ee:ff' -> ('0', 'aa:...'); anything else -> None."""
    parts = line.strip().split()
    if len(parts) == 3 and parts[0] == "btn" and parts[1] in ("0", "1"):
        return parts[1], parts[2]
    return None


def pair_from_events(events):
    """First press-release pair across (level, mac) events -> (mac, pressed, released).

    Events may interleave from multiple units; a pair is two opposite
    levels from the same MAC. Repeats of the same level are ignored.
    """
    first = {}
    for level, mac in events:
        if mac not in first:
            first[mac] = level
        elif first[mac] != level:
            return mac, first[mac], level
    return None


def read_stroke(ser, timeout_s=PROMPT_TIMEOUT_S):
    """Read serial until a complete press-release stroke arrives."""
    events = []
    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        line = ser.readline().decode("utf-8", errors="replace")
        evt = parse_event(line)
        if not evt:
            continue
        events.append(evt)
        pair = pair_from_events(events)
        if pair:
            mac, pressed, released = pair
            print(f"  {mac}: press={pressed} release={released}", file=sys.stderr)
            return mac, pressed, released
    return None


def load_map(path: Path = MAP_PATH) -> dict:
    """{mac: {label, pressed}} from button_map.json; {} if missing."""
    if not path.exists():
        return {}
    data = json.loads(path.read_text())
    return {b["mac"]: b for b in data["buttons"]}


def event_marker(level: str, mac: str, mapping: dict):
    """Normalized marker for a btn event; None if MAC unknown.

    press -> "<label>", release -> "<label>_up", regardless of whether
    the physical switch is active-low or active-high.
    """
    btn = mapping.get(mac)
    if not btn:
        return None
    return btn["label"] if level == str(btn["pressed"]) else f"{btn['label']}_up"


def print_live(ser, mapping):
    """Print mapped events as they arrive until Ctrl-C."""
    print("setup: watching mapped events (Ctrl-C to stop)", file=sys.stderr)
    try:
        while True:
            line = ser.readline().decode("utf-8", errors="replace").strip()
            if not line:
                continue
            parts = line.split()
            if len(parts) == 3 and parts[0] == "btn":
                marker = event_marker(parts[1], parts[2], mapping)
                print(marker if marker else line, flush=True)
            else:
                print(line, flush=True)
    except KeyboardInterrupt:
        print("\nsetup: done", file=sys.stderr)


def selftest():
    assert parse_event("btn 0 aa:bb:cc:dd:ee:ff\n") == ("0", "aa:bb:cc:dd:ee:ff")
    assert parse_event("btn 1 aa:bb:cc:dd:ee:ff") == ("1", "aa:bb:cc:dd:ee:ff")
    assert parse_event("btn 2 x") is None
    assert parse_event("garbage") is None
    # NO button: press=0, release=1
    assert pair_from_events([("0", "m1"), ("1", "m1")]) == ("m1", "0", "1")
    # NC button: press=1, release=0
    assert pair_from_events([("1", "m2"), ("0", "m2")]) == ("m2", "1", "0")
    # stray event from another unit interleaved
    assert pair_from_events([("0", "m1"), ("0", "m3"), ("1", "m1")]) == ("m1", "0", "1")
    # same-level repeat ignored
    assert pair_from_events([("0", "m1"), ("0", "m1"), ("1", "m1")]) == ("m1", "0", "1")
    # single event: no pair yet
    assert pair_from_events([("0", "m1")]) is None
    # event markers: active-low (press=0) and active-high (press=1)
    mapping = {"m1": {"label": "left_hand", "pressed": 0},
               "m2": {"label": "right_foot", "pressed": 1}}
    assert event_marker("0", "m1", mapping) == "left_hand"
    assert event_marker("1", "m1", mapping) == "left_hand_up"
    assert event_marker("1", "m2", mapping) == "right_foot"
    assert event_marker("0", "m2", mapping) == "right_foot_up"
    assert event_marker("1", "unknown", mapping) is None
    print("selftest: OK")


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--port", default="/dev/ttyUSB0")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--buttons", default=",".join(DEFAULT_BUTTONS),
                    help="comma-separated button labels to map")
    ap.add_argument("--monitor", action="store_true",
                    help="don't calibrate, just print raw events")
    ap.add_argument("--selftest", action="store_true")
    args = ap.parse_args()

    if args.selftest:
        selftest()
        return

    labels = [b.strip() for b in args.buttons.split(",") if b.strip()]

    with serial.Serial(args.port, args.baud, timeout=0.5) as ser:
        print(f"setup: connected to {args.port} at {args.baud}", file=sys.stderr)

        if args.monitor:
            print_live(ser, mapping={})
            return

        mapping_list = []
        for label in labels:
            print(f"\n>>> Press {label} now (press and release once)")
            stroke = read_stroke(ser)
            if stroke is None:
                print(f"setup: timed out waiting for {label}, aborting", file=sys.stderr)
                sys.exit(1)
            mac, pressed, _released = stroke
            mapping_list.append({"label": label, "mac": mac, "pressed": int(pressed)})

        mapping = {b["mac"]: b for b in mapping_list}
        MAP_PATH.write_text(json.dumps({"buttons": mapping_list}, indent=2) + "\n")
        print(f"\nsetup: saved {len(mapping_list)} buttons to {MAP_PATH}")
        for b in mapping_list:
            print(f"  {b['label']}: mac={b['mac']} pressed={b['pressed']} "
                  f"(release={1 - b['pressed']})")

        print_live(ser, mapping)


if __name__ == "__main__":
    main()
