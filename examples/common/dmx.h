#pragma once

typedef void* dmx_screen;

dmx_screen init_dmx(int display_index);
void res_dmx(dmx_screen screen, int * width, int * height);
int window_dmx(dmx_screen screen, int width, int height);