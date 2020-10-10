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
link_directories(/opt/vc/lib/)
link_libraries(GLESv2 EGL)
endif ()

link_directories(
	../../lib
)
link_libraries(
	libndi
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
