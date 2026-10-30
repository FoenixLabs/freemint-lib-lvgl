#include "lvgl_app.h"
#include "../src/platform.h"
#include <assert.h>
#include <stdio.h>
static void (*native_close)(void *);static void *native_user;
static unsigned steps,closed,requests;static int fail_open;
static lg_window_t *main_window,*tools;static lv_indev_t *input;
static void (*scenario)(void);
static void read_input(lv_indev_t *in,lv_indev_data_t *data){(void)in;data->state=LV_INDEV_STATE_RELEASED;}
void *lg_platform_open(const char *title,unsigned width,unsigned height,void (*close)(void *),void *user){
 (void)title;if(fail_open)return NULL;
 lv_init();lv_display_create(width,height);input=lv_indev_create();lv_indev_set_read_cb(input,read_input);
 native_close=close;native_user=user;return lv_display_get_default();
}
void lg_platform_step(void *p){(void)p;steps++;assert(steps<8);scenario();}
lv_obj_t *lg_platform_root(void *p){return lv_display_get_screen_active(p);}
void lg_platform_close(void *p){(void)p;lv_deinit();closed++;}
const char *lg_platform_error(void){return "test initialization failure";}
static void refuse(lg_window_t *w,void *user){(void)user;requests++;lg_window_request_close(w);}
static void finish_empty(lg_app_t *app,void *user){(void)user;requests++;lg_app_quit(app);}
static void empty_scenario(void){
 if(steps==1){native_close(native_user);assert(requests==1);}
 if(steps==2){lg_window_destroy(main_window);main_window=NULL;}
 if(steps==3)native_close(native_user); /* Last window did not exit run. */
}
static void windows_scenario(void){
 if(steps==1){native_close(native_user);tools=NULL;} /* Top window only destroyed. */
 if(steps==2)native_close(native_user); /* Main callback explicitly quits. */
}
static void focus_scenario(void){
 if(steps==1){
  lv_obj_t *button=lv_button_create(lg_window_content(main_window));
  lv_indev_send_event(input,LV_EVENT_PRESSED,button); /* Descendant activates main. */
  native_close(native_user);
 }
}
static void protect_close(lg_window_t *w,void *app){lg_app_close(app);lg_window_destroy(w);}
int main(void){
 lg_app_t *app;
 fail_open=1;assert(!lg_app_init("fail"));assert(*lg_app_error());fail_open=0;
 assert(!lg_app_init_size("bad",0,400));
 app=lg_app_init("test");assert(app);assert(!lg_app_init("second"));
 main_window=lg_window_create(app,"Main");assert(main_window);
 lv_obj_update_layout(lg_app_root(app));
 assert(lv_obj_get_width(lv_obj_get_parent(lg_window_content(main_window)))==600);
 lg_window_request_close(main_window);assert(!requests); /* Unhandled close is inert. */
 lg_window_on_close(main_window,refuse,NULL);lg_app_on_close(app,finish_empty,NULL);
 scenario=empty_scenario;assert(!lg_app_run(app));assert(steps==3&&requests==2);
 lg_app_close(app);assert(closed==1);
 app=lg_app_init("windows");assert(app);steps=0;
 main_window=lg_window_create(app,"Main");tools=lg_window_create(app,"Tools");
 lg_window_set_geometry(tools,40,40,320,240);lv_obj_update_layout(lg_app_root(app));
 assert(lv_obj_get_width(lv_obj_get_parent(lg_window_content(tools)))==320);
 lg_window_on_close(main_window,lg_app_quit_callback,app);
 lg_window_on_close(tools,lg_window_destroy_callback,NULL);
 scenario=windows_scenario;assert(!lg_app_run(app));assert(steps==2);lg_app_close(app);
 app=lg_app_init("focus");steps=0;
 main_window=lg_window_create(app,"Main");tools=lg_window_create(app,"Tools");
 lg_window_on_close(main_window,lg_app_quit_callback,app);
 scenario=focus_scenario;assert(!lg_app_run(app));assert(steps==1);lg_app_close(app);
 app=lg_app_init("lifetime");tools=lg_window_create(app,"Tools");
 lg_window_on_close(tools,protect_close,app);lg_window_request_close(tools);
 assert(lg_app_root(app)); /* Closing app inside callback was rejected. */
 main_window=lg_window_create(app,"Main");
 lv_obj_delete(lv_obj_get_parent(lg_window_content(main_window))); /* External LVGL deletion removes handle. */
 lg_app_quit(app);steps=0;assert(!lg_app_run(app));assert(steps==0);lg_app_close(app);
 puts("Application API: initialization, multiple windows, active-window routing, refusal, callback destruction, last-window survival and explicit quit passed");
 return 0;
}
