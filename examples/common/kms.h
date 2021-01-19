#pragma once

typedef void* kms_output;
typedef void* kms_surface;

int init_kms();
void free_kms();
void * get_display_kms();

kms_output create_output_kms(int display_index);
void get_resolution_kms(kms_output output, int * width, int * height);
kms_surface create_surface_kms(int width, int height);
void * get_native_surface_kms(kms_surface surface);
void draw_surface_kms(kms_output output, kms_surface surface, int hdisplay, int vdisplay);