#![no_std]
#![no_main]

#[cfg(not(any(feature = "esp32c6", feature = "esp32s3")))]
compile_error!("Select a target with --features esp32c6 or --features esp32s3");

#[cfg(all(feature = "esp32c6", feature = "esp32s3"))]
compile_error!("Only one target feature may be enabled at a time");

use esp_hal::delay::Delay;
use esp_hal::gpio::{Level, Output, OutputConfig};
use esp_hal::{main, Config};

#[panic_handler]
fn panic(_: &core::panic::PanicInfo) -> ! {
    loop {}
}

#[main]
fn main() -> ! {
    let peripherals = esp_hal::init(Config::default());
    let mut led = Output::new(peripherals.GPIO3, Level::Low, OutputConfig::default());
    let delay = Delay::new();

    loop {
        led.set_high();
        delay.delay_millis(500);
        led.set_low();
        delay.delay_millis(500);
    }
}
