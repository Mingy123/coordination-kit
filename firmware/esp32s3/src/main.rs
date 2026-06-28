#![no_std]
#![no_main]

use esp_backtrace as _;
use esp_hal::{clock::ClockControl, delay::Delay, gpio::IO, peripherals::Peripherals, prelude::*};
use esp_println::println;

#[entry]
fn main() -> ! {
    let peripherals = Peripherals::take();
    let system = peripherals.SYSTEM.split();
    let clocks = ClockControl::boot_defaults(system.clock_control).freeze();
    let io = IO::new(peripherals.GPIO, peripherals.IO_MUX);

    let mut led = io.pins.gpio48.into_push_pull_output(); // ponytail: adjust pin per PCB
    let button = io.pins.gpio47.into_pull_up_input();
    let delay = Delay::new(&clocks);

    println!("coord-s3 ready");

    loop {
        if button.is_low() {
            led.set_high().unwrap();
            delay.delay_ms(100u32);
            led.set_low().unwrap();
        }
        delay.delay_ms(10u32);
    }
}
