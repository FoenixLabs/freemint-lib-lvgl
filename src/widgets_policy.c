#include "lvgl.h"
/* The pinned demo passes its keyboard as user_data to the textarea handlers.
 * Remove those handlers before deleting it, so focus cannot resurrect the
 * keyboard, shrink the tabview, or access a stale object. */
static void detach(lv_obj_t *obj,lv_obj_t *keyboard){
 uint32_t i=lv_obj_get_event_count(obj),n;
 while(i){lv_event_dsc_t *d=lv_obj_get_event_dsc(obj,--i);if(lv_event_dsc_get_user_data(d)==keyboard)lv_obj_remove_event(obj,i);}
 n=lv_obj_get_child_count(obj);for(i=0;i<n;i++)detach(lv_obj_get_child(obj,i),keyboard);
}
void lg_widgets_physical_keyboard(void){
 lv_obj_t *screen=lv_screen_active();uint32_t i=lv_obj_get_child_count(screen);
 while(i){lv_obj_t *child=lv_obj_get_child(screen,--i);if(lv_obj_check_type(child,&lv_keyboard_class)){detach(screen,child);lv_obj_delete(child);}}
}

static void instant_scroll(lv_event_t *e){lv_anim_t *a=lv_event_get_param(e);if(a)lv_anim_set_duration(a,0);}
void lg_widgets_reduce_effects(lv_obj_t *obj){
 uint32_t i;lv_obj_set_style_shadow_width(obj,0,LV_PART_MAIN);
 lv_obj_set_style_anim_duration(obj,0,LV_PART_MAIN);lv_obj_set_style_transition(obj,NULL,LV_PART_MAIN);
 lv_obj_add_event_cb(obj,instant_scroll,LV_EVENT_SCROLL_BEGIN,NULL);
 for(i=0;i<lv_obj_get_child_count(obj);i++)lg_widgets_reduce_effects(lv_obj_get_child(obj,i));
}
