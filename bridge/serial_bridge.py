"""Reads serial events from an ESP32 and publishes them as an LSL string outlet."""
import sys
import serial
from pylsl import StreamInfo, StreamOutlet

SERIAL_PORT = "/dev/ttyUSB0"
BAUD = 115200
STREAM_NAME = "CoordinationKit_Events"

def main(port: str = SERIAL_PORT, baud: int = BAUD):
    info = StreamInfo(STREAM_NAME, "Markers", 1, 0, "string", "coordbridge01")
    outlet = StreamOutlet(info)

    with serial.Serial(port, baud, timeout=1) as ser:
        print(f"bridge: connected to {port} at {baud}", file=sys.stderr)
        while True:
            line = ser.readline().decode("utf-8", errors="replace").strip()
            if line:
                outlet.push_sample([line])

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", default=SERIAL_PORT)
    parser.add_argument("--baud", type=int, default=BAUD)
    args = parser.parse_args()
    main(args.port, args.baud)
