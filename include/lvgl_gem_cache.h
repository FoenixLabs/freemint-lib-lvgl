#ifndef LVGL_GEM_CACHE_H
#define LVGL_GEM_CACHE_H
#include "lvgl.h"
typedef struct lg_scroll_cache lg_scroll_cache;
/* Opt-in for the Widgets Profile page only; owned native32 DIRECT buffer. */
lg_scroll_cache *lg_cache_create(lv_display_t *display,lv_obj_t *page,uint8_t *pixels,unsigned stride,void (*damage)(void *,const lv_area_t *),void *user);
void lg_cache_delete(lg_scroll_cache *cache);
void lg_cache_enable(lg_scroll_cache *cache,int enabled);
unsigned long lg_cache_hits(lg_scroll_cache *cache);
#endif
