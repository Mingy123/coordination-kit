idf_component_register(SRCS "blink.c"
                    INCLUDE_DIRS ".")

# ponytail: chip-specific GPIO default, override with -DBLINK_GPIO=xx
if(${IDF_TARGET} STREQUAL "esp32s3")
    target_compile_definitions(${COMPONENT_LIB} PUBLIC BLINK_GPIO=48)
elseif(${IDF_TARGET} STREQUAL "esp32c6")
    target_compile_definitions(${COMPONENT_LIB} PUBLIC BLINK_GPIO=8)
endif()
