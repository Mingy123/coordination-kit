if(CONFIG_IDF_TARGET_ESP32C6)
    idf_component_register(SRCS "blink_c6.c"
                        INCLUDE_DIRS ".")
else()
    idf_component_register(SRCS "usb_webcam_main.c" "speaker.c" "sd_card.c"
                        INCLUDE_DIRS ".")
endif()
