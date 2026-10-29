#ifndef LVGL_GEM_H
#define LVGL_GEM_H
/* Single AES thread, one fixed-size window. UI belongs to the caller.
 * Call open, create LVGL widgets, repeatedly step, then close.
 * Output is dithered monochrome through VDI's colour-mapped mono raster copy.
 * Native packed colour presentation is deliberately not enabled. */
typedef struct lg_gem lg_gem;
lg_gem *lg_gem_open(unsigned width,unsigned height,const char *title);
int lg_gem_step(lg_gem *ctx); /* 0 after closer/AP_TERM/Escape */
void lg_gem_close(lg_gem *ctx);
#endif
