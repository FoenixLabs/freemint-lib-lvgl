#include "platform.h"
#include "lvgl_gem.h"
void *lg_platform_open(const char *title,unsigned width,unsigned height,void (*close_request)(void *),void *user){
 lg_gem *gem=lg_gem_open(width,height,title);
 if(gem)lg_gem_on_close(gem,close_request,user);
 return gem;
}
void lg_platform_step(void *platform){lg_gem_step(platform);}
lv_obj_t *lg_platform_root(void *platform){return lg_gem_root(platform);}
void lg_platform_close(void *platform){lg_gem_close(platform);}
const char *lg_platform_error(void){return "GEM surface initialization failed (display format, size or allocation)";}
