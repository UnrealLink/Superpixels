CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(OpenCV_STATIC OFF)
FIND_PACKAGE(OpenCV REQUIRED)
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})

PROJECT(SLIC)

SET(CMAKE_CXX_FLAGS "-g")
ADD_EXECUTABLE(slic main.cpp slic.cpp slic.h cielab.cpp cielab.h image.cpp image.h spm.h spm.cpp fusion.h fusion.cpp)
TARGET_LINK_LIBRARIES(slic ${OpenCV_LIBS})
