/*
 * SD card on SPI — DFR1154: CS=GPIO10, MOSI=GPIO11, SCLK=GPIO12, MISO=GPIO13
 */
#include "sd_card.h"
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "sd";
static sdmmc_card_t *s_card;
static bool s_mounted;
static bool s_bus_inited;

/* ponytail: SPI2, one slot, no CD/WP */
#define SPI_HOST  SPI2_HOST
#define PIN_CS    10
#define PIN_MOSI  11
#define PIN_SCLK  12
#define PIN_MISO  13

static esp_err_t init_bus(void)
{
    if (s_bus_inited) return ESP_OK;
    spi_bus_config_t bus = {
        .mosi_io_num   = PIN_MOSI,
        .miso_io_num   = PIN_MISO,
        .sclk_io_num   = PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    esp_err_t ret = spi_bus_initialize(SPI_HOST, &bus, SPI_DMA_CH_AUTO);
    if (ret == ESP_OK) s_bus_inited = true;
    return ret;
}

static esp_err_t mount_internal(const char *mount_point, bool format)
{
    if (s_mounted) return ESP_OK;

    ESP_RETURN_ON_ERROR(init_bus(), TAG, "spi bus");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI_HOST;

    sdspi_device_config_t slot = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot.gpio_cs   = PIN_CS;
    slot.host_id   = SPI_HOST;

    esp_vfs_fat_sdmmc_mount_config_t mcfg = {
        .format_if_mount_failed = format,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
    };

    esp_err_t ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot, &mcfg, &s_card);
    if (ret == ESP_OK) {
        s_mounted = true;
        sdmmc_card_print_info(stdout, s_card);
    }
    return ret;
}

esp_err_t sd_mount(const char *mount_point)
{
    return mount_internal(mount_point, false);
}

esp_err_t sd_format(const char *mount_point)
{
    if (s_mounted) sd_unmount();
    esp_err_t ret = mount_internal(mount_point, true);
    if (ret != ESP_OK) return ret;
    sd_unmount();
    return sd_mount(mount_point);
}

esp_err_t sd_unmount(void)
{
    if (!s_mounted) return ESP_OK;
    s_mounted = false;
    return esp_vfs_fat_sdcard_unmount("/sdcard", s_card);
}
