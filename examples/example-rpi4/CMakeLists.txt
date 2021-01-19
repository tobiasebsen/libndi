cmake_minimum_required(VERSION 3.0)

project(example-rpi4)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
link_libraries(${OPENGL_LIBRARIES})

include_directories(
	../common
	../../include
	/usr/include/libdrm
)

link_directories(
	../../lib
)

link_libraries(
	ndi
	pthread
	drm
	gbm
	EGL
	GLESv2
	avahi-client
	avahi-common
	avcodec
	avformat
	avutil
)

add_definitions(
	-DENABLE_EGL=1
	-DENABLE_KMS=1
)

add_executable(example-rpi4
	main.cpp
	../common/egl.c
	../common/kms.c
	../common/yuv.c

)
