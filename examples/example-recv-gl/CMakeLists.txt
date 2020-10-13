cmake_minimum_required(VERSION 3.0)

project(example-recv-gl)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
set(LINUX 1)
endif ()

include_directories(
	../../include/
)

if (LINUX)
include_directories(/opt/vc/include/)
link_directories(/opt/vc/lib/ /usr/lib/arm-linux-gnueabihf/)
link_libraries(bcm_host GLESv2 EGL avahi-client avahi-common)
endif ()

link_directories(
	../../lib/
)
link_libraries(
	ndi
	avcodec
	avformat
	avutil
)

add_executable(example-recv-gl
	ndi_recv_gl.cpp
	../common/ogl.cpp
	../common/egl.cpp
	../common/glfw.cpp
	../common/yuv.cpp
)
