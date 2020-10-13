cmake_minimum_required(VERSION 3.0)

project(example-find)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	link_directories(/usr/lib/arm-linux-gnueabihf/)
	link_libraries(avahi-client avahi-common)
endif()

include_directories(
	../../include/
)
link_directories(
	../../lib/
)
link_libraries(
	ndi
)

add_executable(example-find
	ndi_find.cpp
)

