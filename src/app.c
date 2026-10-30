#include "lvgl_app.h"
#include "platform.h"
#include <stdlib.h>
struct lg_app {
 void *platform;int quit,running,dispatching;
 lg_app_close_fn close;void *user;
};
static lg_app_t *owner;
static const char *error;
static void request_close(void *user){
 lg_app_t *app=user;
 if(!app->close||app->dispatching)return;
 app->dispatching=1;app->close(app,app->user);app->dispatching=0;
}
lg_app_t *lg_app_init(const char *title){return lg_app_init_size(title,600,400);}
lg_app_t *lg_app_init_size(const char *title,unsigned width,unsigned height){
 lg_app_t *app;error=NULL;
 if(owner){error="An application is already initialized";return NULL;}
 if(!width||!height){error="Surface dimensions must be positive";return NULL;}
 app=calloc(1,sizeof *app);if(!app){error="Cannot allocate application";return NULL;}
 owner=app; /* Also rejects reentrant initialization during backend setup. */
 app->platform=lg_platform_open(title?title:"LVGL",width,height,request_close,app);
 if(!app->platform){error=lg_platform_error();free(app);owner=NULL;return NULL;}
 return app;
}
const char *lg_app_error(void){return error?error:"";}
int lg_app_run(lg_app_t *app){
 if(!app||app->running)return -1;
 app->running=1;while(!app->quit)lg_platform_step(app->platform);
 app->running=0;return 0;
}
void lg_app_quit(lg_app_t *app){if(app)app->quit=1;}
void lg_app_close(lg_app_t *app){
 if(!app)return;
 if(app->running||app->dispatching){error="Use lg_app_quit inside callbacks, then close after run";return;}
 lg_platform_close(app->platform);owner=NULL;free(app);
}
lv_obj_t *lg_app_root(lg_app_t *app){return app?lg_platform_root(app->platform):NULL;}
void lg_app_on_close(lg_app_t *app,lg_app_close_fn callback,void *user){if(app){app->close=callback;app->user=user;}}
void lg_app_quit_callback(lg_app_t *app,void *unused){(void)unused;lg_app_quit(app);}
