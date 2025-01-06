cmake_minimum_required(VERSION 3.22.1)
project("myisp")

set(CMAKE_BUILD_TYPE Debug)

add_library(
        myisp
        SHARED
        native-lib.cpp
        stty.cpp)

find_library(
        log-lib
        log)

target_link_libraries(
        myisp
        ${log-lib})

set_target_properties(myisp PROPERTIES
        COMPILE_FLAGS "-g"
        LINK_FLAGS "-g")