#ifndef LVGL_GEM_H
#define LVGL_GEM_H
#include "lvgl.h"
/* Single AES thread, one native window. UI belongs to the caller.
 * Call open, create LVGL widgets, repeatedly step, then close.
 * Output uses validated native RGB565/XRGB8888 VDI MFDBs, or mono at depth1.
 * Unsupported packing is rejected. All display ownership stays with AES/VDI. */
typedef struct lg_gem lg_gem;
lg_gem *lg_gem_open(unsigned width,unsigned height,const char *title,int resizable);
int lg_gem_step(lg_gem *ctx); /* Pumps events; close requests are callback-only. */
void lg_gem_close(lg_gem *ctx);
void lg_gem_on_close(lg_gem *ctx,void (*callback)(void *),void *user);
lv_obj_t *lg_gem_root(lg_gem *ctx);
#endif
