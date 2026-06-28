"""Reads two hand buttons from FTDI C232HM-EDHSL-0 MPSSE cable.

Pinout:
  GPIOL0 (ADBUS4, bit 0x10) — button 0 (grey wire, pin 6)
  GPIOL1 (ADBUS5, bit 0x20) — button 1
  GND                      — sleeve (black wire, pin 10)
"""
import time
from pyftdi.gpio import GpioAsyncController

GPIO0_MASK = 0x10  # bit 4 — GPIOL0
GPIO1_MASK = 0x20  # bit 5 — GPIOL1

class ButtonReader:
    def __init__(self, url="ftdi://ftdi:232h/1"):
        self.gpio = GpioAsyncController()
        self.gpio.configure(url, direction=0x00, frequency=1e3)

    def read(self):
        """Return (pressed0, pressed1). Active-low: True when pulled to GND."""
        pins = self.gpio.read()
        return (not (pins & GPIO0_MASK), not (pins & GPIO1_MASK))

    def close(self):
        self.gpio.close()

if __name__ == "__main__":
    br = ButtonReader()
    prev = (None, None)
    try:
        while True:
            cur = br.read()
            if cur != prev:
                print(f"[{time.strftime('%H:%M:%S')}] B0={'DOWN' if cur[0] else 'up'}  B1={'DOWN' if cur[1] else 'up'}")
                prev = cur
            time.sleep(0.01)
    except KeyboardInterrupt:
        br.close()
