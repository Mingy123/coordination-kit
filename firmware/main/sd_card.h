#pragma once
#include "esp_err.h"

esp_err_t sd_format(const char *mount_point);
esp_err_t sd_mount(const char *mount_point);
esp_err_t sd_unmount(void);
