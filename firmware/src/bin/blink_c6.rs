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

// ponytail: the esp-bootloader-esp-idf macro lacks #[used] and gets gc'd
//          at opt-level=s. Manual static with #[used] prevents that.
#[used]
#[unsafe(export_name = "esp_app_desc")]
#[unsafe(link_section = ".rodata_desc.appdesc")]
pub static ESP_APP_DESC: esp_bootloader_esp_idf::EspAppDesc =
    esp_bootloader_esp_idf::EspAppDesc::new_internal(
        env!("CARGO_PKG_VERSION"),
        env!("CARGO_PKG_NAME"),
        esp_bootloader_esp_idf::BUILD_TIME,
        esp_bootloader_esp_idf::BUILD_DATE,
        esp_bootloader_esp_idf::ESP_IDF_COMPATIBLE_VERSION,
        0,
        u16::MAX,
        esp_bootloader_esp_idf::MMU_PAGE_SIZE,
    );

#[main]
fn main() -> ! {
    // ponytail: GPIO8 = built-in LED on ESP32-C6-DevKitC-1; change for your PCB
    let config = Config::default().with_cpu_clock(CpuClock::max());
    let peripherals = esp_hal::init(config);

    let mut led = Output::new(peripherals.GPIO8, Level::Low, OutputConfig::default());
    let delay = Delay::new();

    loop {
        led.toggle();
        delay.delay_millis(500);
    }
}
