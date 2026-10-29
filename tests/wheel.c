#include "lvgl_gem_wheel.h"
#include <assert.h>
#include <stdio.h>
static void plain(lv_obj_t *o,int x,int y,int w,int h){
 lv_obj_remove_style_all(o);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);
}
void lg_test_wheel(lv_display_t *display){
 lv_obj_t *old=lv_display_get_screen_active(display),*screen=lv_obj_create(NULL);
 lv_obj_t *box=lv_obj_create(screen),*content=lv_obj_create(box),*inner,*large;
 int axis,n;unsigned dir;
 for(dir=0;dir<4;dir++){
  assert(lg_wheel_decode((dir<<8)|3,&axis,&n));assert(axis==(int)(dir>>1));assert(n==((dir&1)?3:-3));
  assert(lg_wheel_decode((dir<<8)|253,&axis,&n));assert(n==((dir&1)?3:-3));
 }
 assert(!lg_wheel_decode(0,&axis,&n));assert(!lg_wheel_decode(0x401,&axis,&n));
 plain(screen,0,0,600,400);plain(box,20,20,200,180);plain(content,0,0,400,500);
 lv_obj_remove_flag(content,LV_OBJ_FLAG_SCROLLABLE);lv_screen_load(screen);lv_obj_update_layout(screen);
 /* No prior click: hover scrolls the container in both axes and clamps. */
 assert(lg_wheel_scroll(display,50,50,0,1));assert(lv_obj_get_scroll_y(box)==32);
 assert(lg_wheel_scroll(display,50,50,0,-1));assert(lv_obj_get_scroll_y(box)==0);
 assert(lg_wheel_scroll(display,50,50,1,2));assert(lv_obj_get_scroll_x(box)==64);
 assert(lg_wheel_scroll(display,50,50,1,-2));assert(lv_obj_get_scroll_x(box)==0);
 assert(!lg_wheel_scroll(display,-1,50,0,1));assert(lv_obj_get_scroll_y(box)==0);
 assert(!lg_wheel_scroll(display,550,350,0,1));
 assert(lg_wheel_scroll(display,50,50,0,128));assert(lv_obj_get_scroll_bottom(box)==0);
 assert(lg_wheel_scroll(display,50,50,0,-128));assert(lv_obj_get_scroll_y(box)==0);
 inner=lv_obj_create(content);plain(inner,10,10,100,80);
 large=lv_obj_create(inner);plain(large,0,0,100,240);lv_obj_remove_flag(large,LV_OBJ_FLAG_SCROLLABLE);
 lv_obj_update_layout(screen);
 assert(lg_wheel_scroll(display,50,50,0,1));assert(lv_obj_get_scroll_y(inner)==32);assert(lv_obj_get_scroll_y(box)==0);
 lv_obj_scroll_to_y(inner,160,LV_ANIM_OFF);
 lv_obj_remove_flag(inner,LV_OBJ_FLAG_SCROLL_CHAIN_VER);
 assert(!lg_wheel_scroll(display,50,50,0,1));assert(lv_obj_get_scroll_y(box)==0);
 lv_obj_add_flag(inner,LV_OBJ_FLAG_SCROLL_CHAIN_VER);
 assert(lg_wheel_scroll(display,50,50,0,1));assert(lv_obj_get_scroll_y(box)==32);
 lv_screen_load(old);lv_obj_delete(screen);
 puts("Wheel decoding, hover, axes, bounds and nested scroll chaining passed");
}
