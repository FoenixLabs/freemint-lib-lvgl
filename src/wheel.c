#include "lvgl_gem_wheel.h"
int lg_wheel_decode(uint16_t info,int *horizontal,int *steps){
 unsigned orient=(info>>8)&15;int amount=info&255;
 if(orient>3||!amount)return 0;
 if(amount>=128)amount=256-amount;
 *horizontal=(orient&2)!=0;*steps=(orient&1)?amount:-amount;
 return 1;
}
int lg_wheel_scroll(lv_display_t *display,int x,int y,int horizontal,int steps){
 lv_point_t point={x,y};lv_obj_t *obj;lv_dir_t direction;lv_obj_flag_t chain;
 if(!steps||x<0||y<0||x>=lv_display_get_horizontal_resolution(display)||y>=lv_display_get_vertical_resolution(display))return 0;
 /* Keep externally supplied counts bounded, also avoiding multiplication overflow. */
 if(steps>128)steps=128;
 if(steps< -128)steps=-128;
 lv_obj_update_layout(lv_display_get_screen_active(display));
 obj=lv_indev_search_obj(lv_display_get_layer_sys(display),&point);
 if(!obj)obj=lv_indev_search_obj(lv_display_get_layer_top(display),&point);
 if(!obj)obj=lv_indev_search_obj(lv_display_get_screen_active(display),&point);
 direction=horizontal?(steps>0?LV_DIR_RIGHT:LV_DIR_LEFT):(steps>0?LV_DIR_BOTTOM:LV_DIR_TOP);
 chain=horizontal?LV_OBJ_FLAG_SCROLL_CHAIN_HOR:LV_OBJ_FLAG_SCROLL_CHAIN_VER;
 for(;obj;obj=lv_obj_get_parent(obj)){
  if(lv_obj_has_state(obj,LV_STATE_DISABLED))return 0;
  if(lv_obj_has_flag(obj,LV_OBJ_FLAG_SCROLLABLE)&&(lv_obj_get_scroll_dir(obj)&direction)){
   int32_t available=horizontal?(steps>0?lv_obj_get_scroll_right(obj):lv_obj_get_scroll_left(obj)):
                               (steps>0?lv_obj_get_scroll_bottom(obj):lv_obj_get_scroll_top(obj));
   if(available>0){
    /* Immediate bounded movement avoids queuing animation work on the 68060. */
    int32_t dx=horizontal?-steps*32:0,dy=horizontal?0:-steps*32;
    lv_obj_scroll_by_bounded(obj,dx,dy,LV_ANIM_OFF);return 1;
   }
  }
  if(!lv_obj_has_flag(obj,chain))break;
 }
 return 0;
}
