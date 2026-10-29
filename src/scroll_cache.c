#include "lvgl_gem_cache.h"
#include "src/display/lv_display_private.h"
#include "src/core/lv_obj_private.h"
#include "src/core/lv_obj_draw_private.h"
#include "src/misc/lv_area_private.h"
#include <stdlib.h>
#include <string.h>
/* This narrow optimization is tied to the pinned 9.2.2 invalidation sequence:
 * scroll moves children, emits SCROLL, then invalidates the page. Anything else
 * invalidating since the last completed render cancels reuse. */
struct lg_scroll_cache {
 lv_display_t *display;lv_obj_t *page;uint8_t *pixels;unsigned stride;
 int enabled,valid,bad,expected,scrolled,internal;int32_t old_y,old_x;
 lv_area_t old_area;unsigned long hits;
 void (*damage)(void *,const lv_area_t *);void *user;
};
static int same(const lv_area_t *a,const lv_area_t *b){return a->x1==b->x1&&a->y1==b->y1&&a->x2==b->x2&&a->y2==b->y2;}
static int safe(lg_scroll_cache *c,lv_area_t *area){
 lv_obj_t *p=c->page,*screen=lv_display_get_screen_active(c->display);uint32_t i;lv_area_t r;
 lv_obj_get_coords(p,area);
 if(area->x1<0||area->y1<0||area->x2>=lv_display_get_horizontal_resolution(c->display)||area->y2>=lv_display_get_vertical_resolution(c->display))return 0;
 if(lv_obj_get_style_radius(p,0)||lv_obj_get_style_border_width(p,0)||lv_obj_get_scroll_x(p)!=c->old_x)return 0;
 if(lv_obj_get_child_count(lv_display_get_layer_top(c->display))||lv_obj_get_child_count(lv_display_get_layer_sys(c->display)))return 0;
 for(i=0;i<lv_obj_get_child_count(p);i++)if(lv_obj_has_flag(lv_obj_get_child(p,i),LV_OBJ_FLAG_FLOATING))return 0;
 /* Flat untransformed ancestors are required: translating a gradient, texture,
  * translucent layer or rounded clipping boundary would not preserve pixels. */
 for(;p;p=lv_obj_get_parent(p)){
  if(lv_obj_has_flag(p,LV_OBJ_FLAG_HIDDEN)||lv_obj_get_style_bg_grad_dir(p,0)!=LV_GRAD_DIR_NONE||lv_obj_get_style_bg_image_src(p,0)||lv_obj_get_style_opa(p,0)!=255||lv_obj_get_style_transform_rotation(p,0)||lv_obj_get_style_transform_scale_x(p,0)!=256||lv_obj_get_style_transform_scale_y(p,0)!=256)return 0;
 }
 /* Reserve the right96 pixels for the fixed theme button and scrollbars.
  * Any other screen-level overlay touching the reused region cancels reuse. */
 r=*area;r.x2-=96;if(r.x2<r.x1)return 0;
 for(i=1;i<lv_obj_get_child_count(screen);i++){
  lv_obj_t *o=lv_obj_get_child(screen,i);lv_area_t box,hit;
  if(lv_obj_has_flag(o,LV_OBJ_FLAG_HIDDEN))continue;
  lv_obj_get_coords(o,&box);lv_area_increase(&box,lv_obj_get_ext_draw_size(o),lv_obj_get_ext_draw_size(o));
  if(lv_area_intersect(&hit,&r,&box))return 0;
 }
 return 1;
}
static void invalidate(lg_scroll_cache *c,int x1,int y1,int x2,int y2){
 lv_area_t r={x1,y1,x2,y2};if(x2>=x1&&y2>=y1)lv_obj_invalidate_area(lv_display_get_screen_active(c->display),&r);
}
static void event(lv_event_t *e){
 lg_scroll_cache *c=lv_event_get_user_data(e);lv_event_code_t code=lv_event_get_code(e);lv_area_t area;
 if(code==LV_EVENT_DELETE){c->page=NULL;c->valid=0;return;}
 if(!c->page)return;
 if(code==LV_EVENT_SCROLL){c->scrolled=1;c->expected=1;return;}
 if(code==LV_EVENT_INVALIDATE_AREA){
  lv_area_t *r=lv_event_get_param(e);if(c->internal)return;
  lv_obj_get_coords(c->page,&area);
  if(!(c->expected&&same(r,&area)))c->bad=1;
  c->expected=0;return;
 }
 if(code==LV_EVENT_RENDER_READY){
  c->old_y=lv_obj_get_scroll_y(c->page);c->old_x=lv_obj_get_scroll_x(c->page);
  lv_obj_get_coords(c->page,&c->old_area);c->valid=1;c->bad=c->scrolled=c->expected=0;return;
 }
 if(code==LV_EVENT_REFR_START){
  int32_t delta;int y,start,end,step;lv_area_t reused;
  lv_obj_update_layout(lv_display_get_screen_active(c->display));
  if(!c->enabled||!c->valid||c->bad||!c->scrolled||!safe(c,&area)||!same(&area,&c->old_area))return;
  delta=lv_obj_get_scroll_y(c->page)-c->old_y;
  if(!delta||delta>=lv_area_get_height(&area)||-delta>=lv_area_get_height(&area))return;
  reused=area;reused.x2-=96;
  if(delta>0){reused.y2-=delta;start=reused.y1;end=reused.y2+1;step=1;}
  else{reused.y1-=delta;start=reused.y2;end=reused.y1-1;step=-1;}
  for(y=start;y!=end;y+=step)memmove(c->pixels+y*c->stride+reused.x1*4,c->pixels+(y+delta)*c->stride+reused.x1*4,lv_area_get_width(&reused)*4U);
  /* Replace only a known scroll-only invalidation batch. Private access is
   * version-pinned; no dependency files are modified. */
  c->display->inv_p=0;memset(c->display->inv_area_joined,0,sizeof c->display->inv_area_joined);
  c->internal=1;
  invalidate(c,area.x1,area.y1,area.x2,reused.y1-1);
  invalidate(c,area.x1,reused.y2+1,area.x2,area.y2);
  invalidate(c,area.x1,reused.y1,reused.x1-1,reused.y2);
  invalidate(c,reused.x2+1,reused.y1,area.x2,reused.y2);
  c->internal=0;c->hits++;if(c->damage)c->damage(c->user,&area);
 }
}
lg_scroll_cache *lg_cache_create(lv_display_t *d,lv_obj_t *page,uint8_t *pixels,unsigned stride,void (*damage)(void *,const lv_area_t *),void *user){
 lg_scroll_cache *c=calloc(1,sizeof *c);if(!c)return NULL;
 /* Keep Profile's scrollbar appearance constant: the theme's SCROLLED
  * selector otherwise restyles the whole subtree at every scroll start/end. */
 lv_obj_remove_style(page,NULL,LV_PART_SCROLLBAR|LV_STATE_SCROLLED);
 c->display=d;c->page=page;c->pixels=pixels;c->stride=stride;c->enabled=1;c->bad=1;c->damage=damage;c->user=user;
 lv_obj_add_event_cb(page,event,LV_EVENT_SCROLL,c);lv_obj_add_event_cb(page,event,LV_EVENT_DELETE,c);
 lv_display_add_event_cb(d,event,LV_EVENT_INVALIDATE_AREA,c);lv_display_add_event_cb(d,event,LV_EVENT_REFR_START,c);lv_display_add_event_cb(d,event,LV_EVENT_RENDER_READY,c);return c;
}
void lg_cache_delete(lg_scroll_cache *c){
 uint32_t i;if(!c)return;
 if(c->page)lv_obj_remove_event_cb_with_user_data(c->page,event,c);
 i=lv_display_get_event_count(c->display);while(i){lv_event_dsc_t *e=lv_display_get_event_dsc(c->display,--i);if(lv_event_dsc_get_user_data(e)==c)lv_display_delete_event(c->display,i);}
 free(c);
}
void lg_cache_enable(lg_scroll_cache *c,int enabled){if(c){c->enabled=enabled;c->bad=1;}}
unsigned long lg_cache_hits(lg_scroll_cache *c){return c?c->hits:0;}
