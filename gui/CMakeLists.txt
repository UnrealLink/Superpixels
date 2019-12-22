# gtkmm
find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)

if(NOT GTKMM_FOUND)
	message(FATAL_ERROR "gtkmm-3.0 not found!")
endif()

include_directories(${GTKMM_INCLUDE_DIRS})


# utils
include_directories(${CMAKE_SOURCE_DIR}/src)

# library
file(
	GLOB
	src_files
	*.cpp
)

add_library(my_gui
	SHARED
	${src_files})

target_link_libraries(my_gui ${GTKMM_LIBRARIES} utils ${OpenCV_LIBS})

install(TARGETS my_gui
	DESTINATION lib)

file(
	GLOB
	headers
	*.h
)

install(FILES ${headers}
	DESTINATION include/${CMAKE_PROJECT_NAME})
