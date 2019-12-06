include_directories(${CMAKE_SOURCE_DIR}/src)

# color transfer
add_executable(ColorTransfer color_transfer.cpp)
target_link_libraries(ColorTransfer utils ${OpenCV_LIBS})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ColorTransfer
	DESTINATION bin
	RENAME ${CMAKE_PROJECT_NAME}-ColorTransfer)

# GUI to colorize gray images
find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)

if(NOT GTKMM_FOUND)
	message(FATAL_ERROR "gtkmm-3.0 not found!")
endif()

include_directories(${GTKMM_INCLUDE_DIRS})

add_executable(Colorize colorize.cpp)
target_link_libraries(Colorize utils ${GTKMM_LIBRARIES})
