#!/usr/bin/env python3
"""Standalone button-mapping setup for the coordination kit.

Reads raw `btn <level> <mac>` events from the S3 bridge over serial,
prompts the user to press each button once (press + release), and saves
a MAC -> label map to button_map.json. No LSL dependency.

Usage:
  python3 setup.py                        # calibrate left_hand,right_foot
  python3 setup.py --buttons "left_hand,right_foot,left_foot"
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
DEFAULT_BUTTONS = ["left_hand", "right_foot"]
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
            print("monitor: watching events (Ctrl-C to stop)", file=sys.stderr)
            while True:
                line = ser.readline().decode("utf-8", errors="replace").strip()
                if line:
                    print(line, flush=True)
            return

        mapping = []
        for label in labels:
            print(f"\n>>> Press {label} now (press and release once)")
            stroke = read_stroke(ser)
            if stroke is None:
                print(f"setup: timed out waiting for {label}, aborting", file=sys.stderr)
                sys.exit(1)
            mac, pressed, _released = stroke
            mapping.append({"label": label, "mac": mac, "pressed": int(pressed)})

    MAP_PATH.write_text(json.dumps({"buttons": mapping}, indent=2) + "\n")
    print(f"\nsetup: saved {len(mapping)} buttons to {MAP_PATH}")
    for b in mapping:
        print(f"  {b['label']}: mac={b['mac']} pressed={b['pressed']} "
              f"(release={1 - b['pressed']})")


if __name__ == "__main__":
    main()
