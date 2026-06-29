# firmware-rust — Abandoned Rust ESP-HAL Attempt

This directory was the first attempt at writing ESP32 firmware for the
coordination kit using **Rust ESP HAL** (esp-hal v1.0, edition 2024).

## What's here

- **blink-c6** — blinks GPIO8 (built-in LED on ESP32-C6-DevKitC-1) every 500 ms
- **blink-s3** — blinks GPIO3 (built-in LED on ESP32-S3-DevKitC-1) every 500 ms

Both use [`esp-hal`](https://github.com/esp-rs/esp-hal) with the `unstable`
feature flag, `esp-bootloader-esp-idf`, and a custom `build.rs` for linker
script handling.

## Why abandoned

The Rust ESP-IDF ecosystem (as of esp-hal ~1.0) lacked:

1. **USB device (peripheral) support** — needed for UVC webcam
2. **Camera peripheral drivers** — no `esp32-camera` equivalent
3. **I2S support** — needed for MAX98357A speaker amp

These gaps forced a move to the C-based ESP-IDF (`firmware/`), which has mature
support for all three via Espressif's official components and driver libraries.

## Requirements

- Rust `esp` toolchain (installed via `espup`)
- `espflash` for flashing

## Build (for reference — not actively maintained)

```bash
cargo build --features esp32s3 --target xtensa-esp32s3-none-elf --bin blink-s3
cargo build --features esp32c6 --target riscv32imac-unknown-none-elf --bin blink-c6
```

See `firmware/` for the active firmware.
