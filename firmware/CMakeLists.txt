cmake_minimum_required(VERSION 3.16)

# Point to esp-iot-solution for usb_device_uvc component
set(IOT_SOLUTION_PATH $ENV{HOME}/esp/esp-iot-solution)
list(APPEND EXTRA_COMPONENT_DIRS ${IOT_SOLUTION_PATH}/components/usb/usb_device_uvc)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(usb_webcam)
