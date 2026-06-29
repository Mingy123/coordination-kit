if("$ENV{IDF_TARGET}" STREQUAL "esp32s3")
idf_component_register(SRCS "boot_hooks.c")
endif()
