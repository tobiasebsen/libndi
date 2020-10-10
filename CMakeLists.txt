cmake_minimum_required(VERSION 3.0)

project(libndi)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include_directories(
	include/
)

if(LINUX)
	include_directories(
		/opt/vc/include
	)
	link_directories(
		/opt/vc/lib
	)
endif()
if(APPLE)
	include_directories(
		/usr/local/include
	)
	link_directories(
		/usr/local/lib
	)
endif()

link_libraries(
	avcodec
	avformat
	avutil
)

add_library(ndi
	src/recv.c
	src/scramble.c
	src/codec.c
)
