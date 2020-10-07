cmake_minimum_required(VERSION 3.0)

project(example-recv)

include_directories(
	../../include/
)
link_directories(
	../../lib
)
link_libraries(
	libndi
	avcodec
	avformat
	avutil
)

add_executable(example-recv
	ndi_recv.cpp
)

