#ifndef LVGL_GEM_RENDER_H
#define LVGL_GEM_RENDER_H
#include "lvgl.h"
/* Only this owned display buffer uses FF/R/G/B. Intermediate layers and assets
 * retain LVGL's byte layout. NULL unregisters before freeing the buffer. */
void lg_render_native(void *buffer,size_t bytes);
void lg_render_profile(int enabled);
extern unsigned long lg_render_ms[3],lg_render_pixels[3],lg_render_fallbacks;
#endif
