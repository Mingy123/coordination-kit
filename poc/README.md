# poc — Laptop Proof of Concept

Desktop simulation of the button-input pipeline before moving to real ESP32 hardware.

## What it does

Displays a Pygame window with four indicators:
- **B0 / B1** — two physical buttons read from an FTDI C232HM-EDHSL-0 MPSSE cable via pyftdi
- **FL / FR** — keyboard foot switches (keys 2 = left, 4 = right)

All four show ON/OFF in real-time at 60 fps.

## Hardware

- FTDI C232HM-EDHSL-0 MPSSE cable — GPIOL0 (bit 0x10, pin 6, grey) = button 0, GPIOL1 (bit 0x20, pin 5) = button 1
- Buttons are active-low (pulled to GND = pressed)
- No foot-switch hardware needed — just use keyboard keys 2 and 4

## Usage

```bash
pip install -r requirements.txt
python ui.py
```

Or run the raw button reader standalone:

```bash
python button_reader.py
```

## Files

- `button_reader.py` — `ButtonReader` class wrapping pyftdi GPIO polling
- `ui.py` — Pygame main loop
- `requirements.txt` — `pygame`, `pyftdi`
