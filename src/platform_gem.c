#include "platform.h"
#include "lvgl_gem.h"
#include "gem_menu.h"
void *lg_platform_open(const char *title,unsigned width,unsigned height,int resizable,void (*close_request)(void *),void *user){
 lg_gem *gem=lg_gem_open(width,height,title,resizable);
 if(gem)lg_gem_on_close(gem,close_request,user);
 return gem;
}
int lg_platform_menu(void *p,const lg_menu_model *m,void (*selected)(void *,int),void *user){return lg_gem_set_menu(p,m,selected,user);}
void lg_platform_step(void *platform){lg_gem_step(platform);}
lv_obj_t *lg_platform_root(void *platform){return lg_gem_root(platform);}
void lg_platform_close(void *platform){lg_gem_close(platform);}
const char *lg_platform_error(void){return "GEM surface initialization failed (display format, size or allocation)";}
