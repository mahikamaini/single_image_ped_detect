# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(ABSOLUTE_PATH "/Users/mahikamaini/esp/single_image_ped_detect/esp-who/components")
set(EXTRA_COMPONENT_DIRS ${ABSOLUTE_PATH}/who_task
                         ${ABSOLUTE_PATH}/who_peripherals/who_cam
                         ${ABSOLUTE_PATH}/who_peripherals/who_usb
                         ${ABSOLUTE_PATH}/who_peripherals/who_lcd
                         ${ABSOLUTE_PATH}/who_frame_cap
                         ${ABSOLUTE_PATH}/who_detect
                         ${ABSOLUTE_PATH}/who_app/who_detect_app
                         ${ABSOLUTE_PATH}
                         ${ABSOLUTE_PATH}/who_peripherals
                         ${ABSOLUTE_PATH}/who_app)

add_compile_options(-fdiagnostics-color=always)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(DEPENDENCIES_LOCK dependencies.lock.${IDF_TARGET})
project(pedestrian_detect)