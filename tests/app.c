#include "lvgl_app.h"
#include "../src/platform.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void (*native_close)(void *);static void *native_user;
static unsigned steps,closed,requests;static int last_resizable;static int fail_open;
static void (*scenario)(void);
static void (*menu_action)(void *,int);static void *menu_user;static unsigned menu_calls;
int lg_platform_menu(void *p,const lg_menu_model *m,void (*selected)(void *,int),void *user){
 (void)p;assert(m->count>=1&&!strcmp(m->titles[0],"File"));
 assert(m->items[0].menu==LG_MENU_FILE&&!strcmp(m->items[0].label,"Quit"));
 if(m->count==2){assert(!strcmp(m->titles[1],"Edit"));assert(!strcmp(m->items[1].label,"Clear"));}
 menu_action=selected;menu_user=user;return 1;
}
static void menu_callback(lg_app_t *app,void *user){
 assert(user==&menu_calls);menu_calls++;lg_app_close(app);assert(lg_app_root(app));
}
static void menu_scenario(void){
 lg_app_t *app=menu_user;
 if(steps==1){
  assert(!lg_menu_add(app,"Too late"));menu_action(menu_user,0);menu_action(menu_user,999);
  menu_action(menu_user,2);assert(menu_calls==1);menu_action(menu_user,3);assert(menu_calls==1);
 }
 if(steps==2)menu_action(menu_user,LG_MENU_QUIT);
}
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
 app=lg_app_init("Menus",600,400,0);assert(app);steps=0;
 assert(!lg_menu_add(app,""));assert(!lg_menu_add_item(app,99,"Bad",NULL,NULL));
 {char title[]="Edit",label[]="Clear";int edit=lg_menu_add(app,title);assert(edit==2);
  assert(lg_menu_add_item(app,edit,label,menu_callback,&menu_calls)==2);
  assert(lg_menu_add_separator(app,edit)==3);title[0]='X';label[0]='X';}
 assert(!lg_menu_on_item(app,3,menu_callback,NULL));
 scenario=menu_scenario;assert(!lg_app_run(app));assert(steps==2&&menu_calls==1);lg_app_close(app);

 app=lg_app_init_size("Fixed size",320,240);assert(app&&last_resizable==0);lg_app_close(app);lg_app_close(NULL);
 puts("Native surface API: direct root, dimensions, initialization/retry, close refusal/reentrancy, explicit quit and cleanup passed");
 return 0;
}
