#include "lvgl_app.h"
#include "../src/platform.h"
#include <assert.h>
#include <stdio.h>
static void (*native_close)(void *);static void *native_user;
static unsigned steps,closed,requests;static int last_resizable;static int fail_open;
static void (*scenario)(void);
void *lg_platform_open(const char *title,unsigned width,unsigned height,int resizable,void (*close)(void *),void *user){
 (void)title;assert(resizable==0||resizable==1);last_resizable=resizable;if(fail_open)return NULL;
 lv_init();lv_display_create(width,height);
 native_close=close;native_user=user;return lv_display_get_default();
}
void lg_platform_step(void *p){(void)p;steps++;assert(steps<8);scenario();}
lv_obj_t *lg_platform_root(void *p){return lv_display_get_screen_active(p);}
void lg_platform_close(void *p){(void)p;lv_deinit();closed++;}
const char *lg_platform_error(void){return "test initialization failure";}
static void refuse(lg_app_t *app,void *user){
 (void)user;requests++;native_close(native_user); /* Recursive close is ignored. */
 lg_app_close(app); /* Cleanup during callback must be rejected. */
 assert(lg_app_root(app));
}
static void close_scenario(void){
 lg_app_t *app=native_user;
 if(steps==1){native_close(native_user);assert(!requests);lg_app_on_close(app,refuse,NULL);}
 if(steps==2){native_close(native_user);assert(requests==1);lg_app_on_close(app,lg_app_quit_callback,NULL);}
 if(steps==3)native_close(native_user);
}
int main(void){
 lg_app_t *app;
 fail_open=1;assert(!lg_app_init("fail",600,400,0));assert(*lg_app_error());fail_open=0;
 assert(!lg_app_init_size("bad",0,400));
 assert(!lg_app_init("flags",600,400,2));
 assert(!lg_app_init("too small",32,32,LG_APP_RESIZABLE));
 app=lg_app_init("Native surface",600,400,0);assert(app);assert(!lg_app_init("second",600,400,0));
 lv_obj_t *root=lg_app_root(app);
 assert(root==lv_screen_active());assert(lv_obj_get_child_count(root)==0);
 assert(lv_obj_get_width(root)==600&&lv_obj_get_height(root)==400);
 lv_label_set_text(lv_label_create(root),"Direct surface content");
 assert(lv_obj_get_child_count(root)==1); /* No nested window/header/closer. */
 scenario=close_scenario;assert(!lg_app_run(app));assert(steps==3&&requests==1);
 lg_app_close(app);assert(closed==1);
 app=lg_app_init("Custom size",320,240,LG_APP_RESIZABLE);assert(app);assert(last_resizable==1);
 assert(lv_obj_get_width(lg_app_root(app))==320&&lv_obj_get_height(lg_app_root(app))==240);
 lg_app_quit(app);steps=0;assert(!lg_app_run(app));assert(!steps);lg_app_close(app);
 assert(closed==2);
 app=lg_app_init_size("Fixed size",320,240);assert(app&&last_resizable==0);lg_app_close(app);lg_app_close(NULL);
 puts("Native surface API: direct root, dimensions, initialization/retry, close refusal/reentrancy, explicit quit and cleanup passed");
 return 0;
}
