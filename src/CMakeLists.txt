# OpenCV
set(OpenCV_STATIC OFF)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# library
file(
	GLOB
	src_files
	*.cpp
)

add_library(utils
	SHARED
	${src_files})

target_link_libraries(utils ${OpenCV_LIBS})

install(TARGETS utils
	DESTINATION lib)

file(
	GLOB
	headers
	*.h
)

install(FILES ${headers}
	DESTINATION include/${CMAKE_PROJECT_NAME})
