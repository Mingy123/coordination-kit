# DFRobot DFR1154 — ESP32-S3 AI Camera
## Full component & pinout reference

**Source:** `Schematic_v1_0.pdf` — Title: *ESP32-S3 AI Camera*, Code: DFR1154, Revision: V1.0, Drawn by YXD, dated 2024/9/23.

This document catalogs every functional block on the board, the part number behind it, and the exact pin-to-pin wiring back to the ESP32-S3 module, derived directly from the schematic.

---

## 1. Core MCU — ESP32-S3-R8 (U3)

The ESP32-S3-R8 (8MB embedded PSRAM variant) is the central module. All other ICs on the board are peripherals wired into its GPIOs. Pin numbers below refer to the physical pin numbers silkscreened on the schematic symbol for U3.

| Physical pin | Pin name (silkscreen) | GPIO | Net / function |
|---|---|---|---|
| 1 | LNA_IN | — | RF input from antenna matching network |
| 2, 3 | VDD3P3 | — | 3V3 power |
| 4 | CHIP_PU | — | RST (reset, active low) |
| 5 | GPIO0 | GPIO0 | BOOT (strapping pin) |
| 6 | GPIO1 | GPIO1 | Camera VSYNC |
| 7 | GPIO2 | GPIO2 | Camera HSYNC |
| 8 | GPIO3 | GPIO3 | LED (status LED driver, D6) |
| 9 | GPIO4 | GPIO4 | Camera Y9 |
| 10 | GPIO5 | GPIO5 | Camera XMCLK |
| 11 | GPIO6 | GPIO6 | Camera Y8 |
| 12 | GPIO7 | GPIO7 | Camera Y7 |
| 13 | GPIO8 | GPIO8 | SDA (shared I2C: camera SCCB + light sensor) |
| 14 | GPIO9 | GPIO9 | SCL (shared I2C: camera SCCB + light sensor) |
| 15 | GPIO10 | GPIO10 | microSD SD_CS |
| 16 | GPIO11 | GPIO11 | microSD MOSI |
| 17 | GPIO12 | GPIO12 | microSD SCLK |
| 18 | GPIO13 | GPIO13 | microSD MISO |
| 19 | GPIO14 | GPIO14 | Camera Y6 |
| 20 | VDD3P3_RTC | — | 3V3 power (RTC domain) |
| 21 | XTAL_32K_P | — | Camera PCLK |
| 22 | XTAL_32K_N | — | Camera Y2 |
| 23 | GPIO17 | GPIO17 | Camera Y5 |
| 24 | GPIO18 | GPIO18 | Camera Y3 |
| 25 | GPIO19 | GPIO19 | USB_N (USB-C D−) |
| 26 | GPIO20 | GPIO20 | USB_P (USB-C D+) |
| 27 | GPIO21 | GPIO21 | Camera Y4 |
| 28 | SPICS1 | — | (unused / not stuffed) |
| 29 | VDD_SPI | — | SPI flash power |
| 30 | SPIHD | — | SPI flash hold |
| 31 | SPIWP | — | SPI flash write-protect |
| 32 | SPICS0 | — | SPI flash chip-select |
| 33 | SPICLK | — | SPI flash clock |
| 34 | SPIQ | — | SPI flash data (Q) |
| 35 | SPID | — | SPI flash data (D) |
| 36 | SPICLK_N | — | LTR_INT (light sensor interrupt) |
| 37 | SPICLK_P | — | IR_CT (IR LED PWM control) |
| 38 | GPIO33 | GPIO33 | (unused / not stuffed) |
| 39 | GPIO34 | GPIO34 | (unused / not stuffed) |
| 40 | GPIO35 | GPIO35 | (unused / not stuffed) |
| 41 | GPIO36 | GPIO36 | (unused / not stuffed) |
| 42 | GPIO37 | GPIO37 | (unused / not stuffed) |
| 43 | GPIO38 | GPIO38 | PDM_CLK (microphone clock) |
| 45 | MTCK | — | PDM_DATA (microphone data) |
| 46 | MTDO | — | SD_MODE# (speaker amp shutdown/mode select) |
| 47 | MTDI | — | GAIN (speaker amp gain select) |
| 48 | MTMS | — | DIN (speaker amp I2S data in) |
| 49 | UOTXD | — | TXD (UART0 — debug header) |
| 50 | UORXD | — | RXD (UART0 — debug header) |
| 51 | GPIO45 | GPIO45 | BCLK (speaker amp I2S bit clock) |
| 52 | GPIO46 | GPIO46 | LRCLK (speaker amp I2S word-select clock) |
| 53, 54 | XTAL_P, XTAL_N | — | 40MHz crystal (X1) |
| 55, 56 | VDDA | — | Analog 3V3 power |
| 57 | GND | — | Ground |

**Notes:**
- Pins 43, 45, 46, 47, 48 are the chip's JTAG/strapping pins (GPIO38, MTCK, MTDO, MTDI, MTMS), reused here for the microphone and speaker. JTAG debugging will conflict with audio if ever needed.
- The camera and microSD interfaces use entirely separate pins, so both peripherals can operate concurrently.
- 40MHz crystal: X1, with C3/C4 (12pF/50V) load capacitors and R4/R5 (0Ω) series resistors.
- Antenna network: E1 (PCB/chip antenna) → C25 (1pF) → C9 (NG, not stuffed) / C10 (1.8pF) matching network → LNA_IN (pin 1).

---

## 2. Camera sensor connector (P2)

A 24-pin FPC connector carrying the DVP parallel video bus and SCCB control bus out to the camera sensor module. This is the direct sensor-side pinout (not the ESP32 side).

| P2 pin | Signal | Notes |
|---|---|---|
| 1 | NC | |
| 2 | AGND | Analog ground |
| 3 | SDA | SCCB data — shared with ESP32 GPIO8 |
| 4 | AVDD | 2.8V analog supply (from U7) |
| 5 | SCL | SCCB clock — shared with ESP32 GPIO9 |
| 6 | RST | Pulled up to AVDD_2V8 via R14 (5.1KΩ) |
| 7 | VSYNC | → ESP32 GPIO1 |
| 8 | PWDN | Pulled down to GND via R16 (5.1KΩ) — sensor kept out of powerdown |
| 9 | HSYNC | → ESP32 GPIO2 |
| 10 | DVDD | 1.5V digital supply (from U9) |
| 11 | DOVDD | Tied to AVDD_2V8 (I/O domain voltage) |
| 12 | Y9 | → ESP32 GPIO4 |
| 13 | XMCLK | → ESP32 GPIO5 (master clock to sensor) |
| 14 | Y8 | → ESP32 GPIO6 |
| 15 | GND | |
| 16 | Y7 | → ESP32 GPIO7 |
| 17 | PCLK | → ESP32 pin 21 (XTAL_32K_P) |
| 18 | Y6 | → ESP32 GPIO14 |
| 19 | Y2 | → ESP32 pin 22 (XTAL_32K_N) |
| 20 | Y5 | → ESP32 GPIO17 |
| 21 | Y3 | → ESP32 GPIO18 |
| 22 | Y4 | → ESP32 GPIO21 |
| 23 | NC | |
| 24 | NC | |
| 0 | GND | |

**Camera power supplies:**
| Regulator | Part | Input | Output | Powers |
|---|---|---|---|---|
| U7 | ME6206A28XG | 3V3 | AVDD_2V8 | Camera analog/I-O domain (AVDD, DOVDD) |
| U9 | ME6206A15XG | 3V3 | DVDD_1V5 | Camera digital core domain (DVDD) |

---

## 3. microSD card socket (M1)

Standard microSD push-push socket, wired in SPI mode.

| M1 pin | Function | Connects to |
|---|---|---|
| 1 | DAT2 / NC | Not connected |
| 2 | DAT3 / CS | SD_CS → ESP32 GPIO10 |
| 3 | CMD / MOSI | MOSI → ESP32 GPIO11 |
| 4 | VDD | 3V3 |
| 5 | SCLK | SCLK → ESP32 GPIO12 |
| 6 | VSS | GND |
| 7 | DAT0 / MISO | MISO → ESP32 GPIO13 |
| 8 | DAT1 / NC | Not connected |
| 9 | CD | Card-detect switch (mechanical, socket-internal) |
| 10 | SHELL | GND (chassis/shield) |

Decoupling: C31 (100nF/50V) on VDD.

---

## 4. Microphone — PDM mic (U1, MSM261DGT003)

| U1 pin | Function | Connects to |
|---|---|---|
| 1 | VDD | 3V3 |
| 2 | GND | GND |
| 3 | DATA | PDM_DATA → ESP32 MTCK (pin 45) |
| 4 | CLK | PDM_CLK → ESP32 GPIO38 (pin 43) |
| 5 | GND | GND |
| 6 | L/R | Tied to GND (selects left-channel output) |

Decoupling: C1 (100nF/50V), C5 (10µF/10V) on VDD.

---

## 5. Speaker amplifier — I2S Class-D amp (U10, MAX98357A) + connector P1

| U10 pin | Function | Connects to |
|---|---|---|
| 1 | DIN | I2S data in → ESP32 MTMS (pin 48) |
| 2 | GAIN_SLOT | GAIN, set via R6 (100KΩ) |
| 3 | GND | GND |
| 4 | SD_MODE# | → ESP32 MTDO (pin 46); also pulled to 3V3 via R1 (100KΩ) |
| 5 | N.C. | — |
| 6 | N.C. | — |
| 7, 8 | VDD | 3V3 |
| 9 | OUTP | → Speaker connector P1 pin 1 |
| 10 | OUTN | → Speaker connector P1 pin 2 |
| 11, 15 | GND | GND |
| 12, 13 | N.C. | — |
| 14 | LRCLK | I2S word-select → ESP32 GPIO46 (pin 52) |
| 16 | BCLK | I2S bit clock → ESP32 GPIO45 (pin 51) |
| 17 (EP) | GND | Exposed pad, GND |

Decoupling: C44 (100nF/50V), C45 (10µF/10V) on VDD.

**P1 — speaker connector (2-pin):**
| P1 pin | Signal |
|---|---|
| 1 | OUTP |
| 2 | OUTN |

---

## 6. Ambient light sensor (U2, LTR-308ALS-01)

I2C ambient light sensor, sharing the same SDA/SCL bus as the camera's SCCB interface.

| U2 pin | Function | Connects to |
|---|---|---|
| 1 | VDD | 3V3 |
| 2 | NC | — |
| 3 | GND | GND |
| 4 | SCL | Shared I2C clock → ESP32 GPIO9 |
| 5 | INT | LTR_INT → ESP32 pin 36 (SPICLK_N) |
| 6 | SDA | Shared I2C data → ESP32 GPIO8 |

Bus pull-ups: R9, R10, R11 (each 10KΩ to 3V3) on SDA, LTR_INT, and SCL respectively.
Decoupling: C11 (10µF/10V), C12 (100nF/50V) on VDD.

---

## 7. IR illumination LEDs + driver

For night-vision / low-light imaging. Four IR LEDs in series, driven by a boost converter, PWM-controlled by the ESP32.

**Driver — U6 (SY7200AABC), boost converter:**
| U6 pin | Function | Connects to |
|---|---|---|
| 1 | LX | Switch node → inductor L2 (6.8µH) |
| 2 | GND | GND |
| 3 | FB | Feedback, sensed across current-sense resistor R7 |
| 4 | EN/PWM | IR_CT → ESP32 pin 37 (SPICLK_P) — PWM brightness control |
| 5 | OVP | Over-voltage protection, tied to LX/output node |
| 6 | IN | 3V3 |

**LED string:** D1 → D2 → D3 → D4 (all MHP3528IRCT-D, IR emitters) wired in series from the boost converter output (through Schottky diode D5, SS34) to current-sense resistor R7 (6.8Ω/1%) to GND.

Decoupling: C43 (10µF/10V), C14 (100nF/50V) on input; C48 (100nF/50V), C49 (10µF/25V) on boost output.

---

## 8. Status LED (D6)

A single red indicator LED, driven directly from a GPIO.

| Component | Connects to |
|---|---|
| D6 (anode) | LED net → ESP32 GPIO3 (pin 8), through R15 (2.2KΩ) |
| D6 (cathode) | GND |

---

## 9. SPI flash (U4, W25Q128JVPIQ)

128Mbit (16MB) external SPI NOR flash for firmware/program storage, on the ESP32's dedicated SPI flash bus.

| U4 pin | Function | Connects to |
|---|---|---|
| 1 | CS# | ESP32 SPICS0 (pin 32) |
| 2 | IO1 | ESP32 SPIQ (pin 34) |
| 3 | IO2 | ESP32 SPIWP (pin 31) |
| 4 | VSS | GND |
| 5 | IO0 | ESP32 SPID (pin 35) |
| 6 | SCLK | ESP32 SPICLK (pin 33) |
| 7 | IO3 | ESP32 SPIHD (pin 30) |
| 8 | VCC | ESP32 VDD_SPI (pin 29) |

---

## 10. USB-C connector (U5) + ESD protection

USB Type-C receptacle used for both power delivery and serial/programming access.

| U5 pin | Function | Notes |
|---|---|---|
| A1, B12 | GND | |
| A4, B9 | VBUS | → VUSB rail |
| A5 | CC1 | Through R17 (5.1KΩ) to GND — sets device/UFP mode |
| A6 | DP1 | USB_P → ESP32 GPIO20 |
| A7 | DN1 | USB_N → ESP32 GPIO19 |
| A8 | SBU1 | Not connected |
| A9, B4 | VBUS | → VUSB rail |
| A12, B1 | GND | |
| B5 | CC2 | Through R18 (5.1KΩ) to GND |
| B6 | DP2 | USB_P (mirrors A6) |
| B7 | DN2 | USB_N (mirrors A7) |
| B8 | SBU2 | Not connected |

**ESD protection (Q4, Q5 — ESD5V0B03-1006, TVS diode array):** clamp USB_N/USB_P to GND, located near test points T3/T4.

---

## 11. Power input connector (P4) + power OR-ing

External DC barrel/2-pin power input, OR'd with USB and the upstream VCC rail through Schottky diodes so the board can be powered from either source.

| P4 pin | Function |
|---|---|
| 1 | Vin (5–12V) |
| 2 | GND |

| Diode | From | To |
|---|---|---|
| D7 (SS34) | P4 / Vin | Combined VCC node (pre-regulator) |
| D8 (SS34) | VCC (board-level rail) | Combined VCC node |
| D9 (SS34) | VUSB (from USB-C VBUS) | Combined VCC node |

---

## 12. Main buck regulator (U8, JW3651)

Steps down the OR'd input (5–12V or VUSB) to the board's main 3V3 rail.

| U8 pin | Function | Notes |
|---|---|---|
| 1 | CSP | Current-sense (+) |
| 2 | VIN | Combined input from D7/D8/D9 |
| 3 | PGND | Power ground |
| 4 | VO | 3V3 output rail (labeled "3V3" throughout schematic) |
| 5 | FB | Feedback — divider R24 (27K)/R25 (10K) sets Vout; FB=0.9V → Vout=3.33V |
| 6 | OLIM | Current limit, set by R26 (40.2KΩ) |
| 7 | EN | Enable, pulled up via R2 (10KΩ) |
| 8 | BST1 | Bootstrap (switching node 1) |
| 9 | SW1 | Switch node 1 → inductor L1 (4.7µH) |
| 10 | SW2 | Switch node 2 → inductor L1 |
| 11 | BST2 | Bootstrap (switching node 2) |
| 12 | TEST | Tied to GND |
| 13 | VCC | Internal logic supply, from VLDO, decoupled with C41 (10µF/10V) |
| 14 | GND | GND |
| 15 | CSN | Current-sense (−) |

Input bulk caps: C34, C35 (10µF/25V each), C39 (100nF/50V). Output bulk caps: C36 (10µF/10V). Inductor L1: 4.7µH, flanked by C32/C33 (100nF/50V) snubber caps.

---

## 13. User input buttons

| Ref | Function | Wiring |
|---|---|---|
| S1 | Reset (RST) | Momentary switch, pulls CHIP_PU/RST low; pulled up to 3V3 via R12 (10KΩ); debounced by C16/C17 (100nF each) |
| S2 | Boot (download mode) | Momentary switch, pulls GPIO0/BOOT low; pulled up to 3V3 via R13 (10KΩ) |

---

## 14. UART debug/programming header (P3)

4-pin header exposing the ESP32's UART0 for flashing/debugging, separate from USB.

| P3 pin | Function | Notes |
|---|---|---|
| 1 | TX | → TXD net → ESP32 UOTXD (pin 49), through R22 (1KΩ) series resistor |
| 2 | (unlabeled / not stuffed) | |
| 3 | RX | → RXD net → ESP32 UORXD (pin 50), through D10 (1N5819W) and pulled up via R20 (10KΩ) to 3V3 |
| 4 | VCC | 3V3 |

---

## 15. Mounting hardware

| Ref | Type |
|---|---|
| MH1, MH2, MH3, MH4 | Mounting holes |
| FD1, FD2, FD3, FD4 | Fiducial markers (for pick-and-place / optical alignment) |

---

## Board-level net summary (power rails)

| Rail | Source | Used by |
|---|---|---|
| VUSB | USB-C VBUS | Power-OR input to U8 |
| VCC (unregulated in) | Upstream board rail / P4 | Power-OR input to U8 |
| 3V3 | U8 (JW3651) output | ESP32-S3, microSD, mic, speaker amp, light sensor, SPI flash, IR driver input, camera LDO inputs (U7/U9) |
| AVDD_2V8 | U7 (ME6206A28XG) | Camera sensor analog + I/O domain |
| DVDD_1V5 | U9 (ME6206A15XG) | Camera sensor digital core |

---

## Quick peripheral-to-GPIO cross-reference

| Peripheral | ESP32 pins used |
|---|---|
| Camera (DVP + SCCB) | GPIO1, GPIO2, GPIO3, GPIO4, GPIO5, GPIO6, GPIO7, GPIO8, GPIO9, GPIO14, GPIO17, GPIO18, GPIO21, pin 21, pin 22 |
| microSD (SPI) | GPIO10, GPIO11, GPIO12, GPIO13 |
| Light sensor (I2C, shared bus) | GPIO8, GPIO9, pin 36 (INT) |
| Microphone (PDM) | GPIO38, MTCK (pin 45) |
| Speaker amp (I2S) | GPIO45, GPIO46, MTMS (pin 48), MTDO (pin 46), MTDI (pin 47) |
| IR LED driver (PWM) | pin 37 (SPICLK_P) |
| Status LED | GPIO3 |
| USB | GPIO19, GPIO20 |
| UART0 (debug header) | pin 49 (TX), pin 50 (RX) |
| SPI flash | pins 29–35 (dedicated SPI flash bus) |
| Reset / Boot buttons | CHIP_PU (pin 4), GPIO0 (pin 5) |
