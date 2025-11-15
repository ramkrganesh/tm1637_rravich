cmake_minimum_required(VERSION 3.10)
project(tm1637 NONE)

set(SKETCH_PATH "${CMAKE_SOURCE_DIR}/")
set(ARDUINO_FQBN "arduino:avr:nano")

add_custom_target(arduino_compile ALL
    COMMAND arduino-cli compile --fqbn ${ARDUINO_FQBN} ${SKETCH_PATH}
    WORKING_DIRECTORY ${SKETCH_PATH}
    COMMENT "Compiling Arduino sketch using arduino-cli...")
