#ifndef LVGL_GEM_H
#define LVGL_GEM_H
/* Single AES thread, one fixed-size window. UI belongs to the caller.
 * Call open, create LVGL widgets, repeatedly step, then close.
 * Output uses validated native RGB565/XRGB8888 VDI MFDBs, or mono at depth1.
 * Unsupported packing is rejected. All display ownership stays with AES/VDI. */
typedef struct lg_gem lg_gem;
lg_gem *lg_gem_open(unsigned width,unsigned height,const char *title);
int lg_gem_step(lg_gem *ctx); /* 0 after closer/AP_TERM/Escape */
void lg_gem_close(lg_gem *ctx);
#endif
