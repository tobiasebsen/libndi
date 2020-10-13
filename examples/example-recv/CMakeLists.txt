cmake_minimum_required(VERSION 3.0)

project(example-recv)

if(APPLE)
	include_directories(
		/usr/local/include
	)
	link_directories(
		/usr/local/lib
	)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../../include/
)
link_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/
)

link_libraries(
	ndi
	avcodec
	avformat
	avutil
)

add_executable(example-recv
	ndi_recv.cpp
)

