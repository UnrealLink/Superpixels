cmake_minimum_required(VERSION 2.6)

# project configuration
project(Superpixels)

add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(gui)

set(CMAKE_CXX_FLAGS "-g")

