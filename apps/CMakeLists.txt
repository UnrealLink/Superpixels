include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/gui)


# color transfer
add_executable(ColorTransfer color_transfer.cpp)
target_link_libraries(ColorTransfer utils ${OpenCV_LIBS})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ColorTransfer
	DESTINATION bin
	RENAME ${CMAKE_PROJECT_NAME}-ColorTransfer)


# GUI to colorize gray images
include_directories(${GTKMM_INCLUDE_DIRS})
add_executable(Colorize colorize.cpp)
target_link_libraries(Colorize my_gui ${GTKMM_LIBRARIES})
