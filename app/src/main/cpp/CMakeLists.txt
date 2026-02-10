cmake_minimum_required(VERSION 3.22.1)
project(nativecamera)

add_library(nativecamera SHARED native-lib.cpp)

target_link_libraries(nativecamera
        android
        camera2ndk
        mediandk
        log)
