#pragma once

int yuv_init();
void yuv_bind();
void yuv_unbind();
void yuv_view(float left, float right, float top, float bottom, float near, float far);
void yuv_data(int plane, unsigned char * data, int width, int height, int linesize);
void yuv_draw(float x, float y, float width, float height);
