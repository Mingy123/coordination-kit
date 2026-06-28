#![no_std]
#![no_main]

use esp_hal::clock::CpuClock;
use esp_hal::delay::Delay;
use esp_hal::gpio::{Level, Output, OutputConfig};
use esp_hal::{main, Config};

#[panic_handler]
fn panic(_: &core::panic::PanicInfo) -> ! {
    loop {}
}

esp_bootloader_esp_idf::esp_app_desc!();

#[main]
fn main() -> ! {
    // ponytail: GPIO48 = built-in LED on ESP32-S3-DevKitC-1; change for your PCB
    let config = Config::default().with_cpu_clock(CpuClock::max());
    let peripherals = esp_hal::init(config);

    let mut led = Output::new(peripherals.GPIO48, Level::Low, OutputConfig::default());
    let delay = Delay::new();

    loop {
        led.toggle();
        delay.delay_millis(500);
    }
}
