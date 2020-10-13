cmake_minimum_required(VERSION 3.0)

project(libndi)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include_directories(
	include/
)

set(NDI_SOURCES "src/recv.c" "src/scramble.c" "src/codec.c")


if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	include_directories(/opt/vc/include /usr/include)
	link_directories(/opt/vc/lib)
	list(APPEND NDI_SOURCES "src/find_avahi.c")
endif()
if(APPLE)
	include_directories(
		/usr/local/include
	)
	link_directories(
		/usr/local/lib
	)
	list(APPEND NDI_SOURCES "src/find_dnssd.c")
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
	${NDI_SOURCES}
)
