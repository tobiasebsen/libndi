cmake_minimum_required(VERSION 3.0)

project(libndi)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include_directories(
	include/
)

add_library(libndi
	src/recv.c
	src/scramble.c
)
