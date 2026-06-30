if(CONFIG_IDF_TARGET_ESP32C6)
    idf_component_register(SRCS "main_c6.c"
                        INCLUDE_DIRS ".")
else()
    idf_component_register(SRCS "main_s3.c" "usb_webcam.c" "speaker.c" "sd_card.c" "esp_now_bridge.c"
                        INCLUDE_DIRS ".")
endif()
