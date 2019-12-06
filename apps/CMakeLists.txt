include_directories(${CMAKE_SOURCE_DIR}/src)

# color transfer
add_executable(ColorTransfer color_transfer.cpp)
target_link_libraries(ColorTransfer utils ${OpenCV_LIBS})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ColorTransfer
	DESTINATION bin
	RENAME ${CMAKE_PROJECT_NAME}-ColorTransfer)

# GUI to colorize gray images
