/*
 * Bootloader hook: disable USB-Serial-JTAG D+ pullup so the TinyUSB
 * UVC device (not the JTAG port) claims the USB D+/D- lines.
 */

#include "soc/rtc_cntl_struct.h"
#include "soc/usb_serial_jtag_reg.h"

void bootloader_hooks_include(void)
{
}

void bootloader_before_init(void)
{
    SET_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_PAD_PULL_OVERRIDE);
    CLEAR_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_DP_PULLUP);
    CLEAR_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_USB_PAD_ENABLE);
}
