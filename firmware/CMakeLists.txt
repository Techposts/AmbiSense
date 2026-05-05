cmake_minimum_required(VERSION 3.16)

# AmbiSense v6 — ESP-IDF firmware
#
# Build with:
#   . $IDF_PATH/export.sh
#   idf.py set-target esp32c3       # or esp32, esp32s3, esp32c6
#   idf.py build flash monitor

set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ambisense)
