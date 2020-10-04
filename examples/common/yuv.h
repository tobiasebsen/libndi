#pragma once

int yuv_init();
void yuv_bind();
void yuv_unbind();
int yuv_get_uniform(const char * name);
int yuv_get_attrib(const char * name);
void yuv_textures(int tex_y, int tex_u, int tex_v);
