#include "platform.h"
/* Reserved backend: no register access, input, timer or display setup yet.
 * MCP will supply a full-screen root and poll hardware in step(). */
void *lg_platform_open(const char *title,unsigned width,unsigned height,int resizable,void (*close_request)(void *),void *user){
 (void)resizable;(void)title;(void)width;(void)height;(void)close_request;(void)user;return NULL;
}
void lg_platform_step(void *platform){(void)platform;}
lv_obj_t *lg_platform_root(void *platform){(void)platform;return NULL;}
void lg_platform_close(void *platform){(void)platform;}
const char *lg_platform_error(void){return "MCP backend is not implemented";}

int lg_platform_menu(void *p,const lg_menu_model *m,void (*selected)(void *,int),void *user){(void)p;(void)m;(void)selected;(void)user;return 0;}
