#include "lvgl_app.h"
#include "platform.h"
#include <stdlib.h>
struct lg_app {
 void *platform;lg_window_t *windows,*active;int quit,running,dispatching;
 lg_app_close_fn close;void *user;
};
struct lg_window {
 lg_app_t *app;lg_window_t *next;lv_obj_t *object;
 lg_window_close_fn close;void *user;int requesting;
};
static lg_app_t *owner;
static const char *error;
static void request_close(void *user){
 lg_app_t *app=user;
 if(app->active)lg_window_request_close(app->active);
 else if(app->close){app->dispatching++;app->close(app,app->user);app->dispatching--;}
}
static void raise_window(lg_window_t *w){
 lg_window_t **p=&w->app->windows;
 while(*p&&*p!=w)p=&(*p)->next;
 if(*p){*p=w->next;w->next=w->app->windows;w->app->windows=w;}
 w->app->active=w;lv_obj_move_foreground(w->object);
}
static void input_pressed(lv_event_t *e){
 lg_app_t *app=lv_event_get_user_data(e);lv_obj_t *obj=lv_event_get_param(e);
 for(;obj;obj=lv_obj_get_parent(obj)){
  lg_window_t *w;for(w=app->windows;w;w=w->next)if(w->object==obj){raise_window(w);return;}
 }
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
 {lv_indev_t *input=NULL;while((input=lv_indev_get_next(input)))lv_indev_add_event_cb(input,input_pressed,LV_EVENT_PRESSED,app);}
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
 while(app->windows)lg_window_destroy(app->windows);
 lg_platform_close(app->platform);owner=NULL;free(app);
}
lv_obj_t *lg_app_root(lg_app_t *app){return app?lg_platform_root(app->platform):NULL;}
void lg_app_on_close(lg_app_t *app,lg_app_close_fn callback,void *user){if(app){app->close=callback;app->user=user;}}
static void deleted(lv_event_t *e){
 lg_window_t *w=lv_event_get_user_data(e),**p=&w->app->windows;
 while(*p&&*p!=w)p=&(*p)->next;
 if(*p)*p=w->next;
 if(w->app->active==w)w->app->active=w->app->windows;
 free(w);
}
static void activate(lv_event_t *e){
 lg_window_t *w=lv_event_get_user_data(e);
 raise_window(w);
}
static void close_clicked(lv_event_t *e){lg_window_request_close(lv_event_get_user_data(e));}
static void drag(lv_event_t *e){
 lg_window_t *w=lv_event_get_user_data(e);lv_indev_t *input=lv_indev_active();lv_point_t delta;
 if(!input)return;
 lv_indev_get_vect(input,&delta);
 /* Keep the title bar reachable within the surface. */
 int x=lv_obj_get_x(w->object)+delta.x,y=lv_obj_get_y(w->object)+delta.y;
 int max_x=lv_obj_get_width(lg_app_root(w->app))-32;
 int max_y=lv_obj_get_height(lg_app_root(w->app))-32;
 if(max_x<0)max_x=0;
 if(max_y<0)max_y=0;
 if(x<0)x=0;
 if(y<0)y=0;
 if(x>max_x)x=max_x;
 if(y>max_y)y=max_y;
 lv_obj_set_pos(w->object,x,y);
}
lg_window_t *lg_window_create(lg_app_t *app,const char *title){
 lg_window_t *w;lv_obj_t *header,*button;
 if(!app)return NULL;
 w=calloc(1,sizeof *w);if(!w){error="Cannot allocate window";return NULL;}
 w->app=app;w->object=lv_win_create(lg_app_root(app));
 if(!w->object){free(w);error="Cannot create LVGL window";return NULL;}
 lv_obj_set_size(w->object,LV_PCT(100),LV_PCT(100));
 if(app->windows){lv_obj_set_size(w->object,LV_PCT(75),LV_PCT(75));lv_obj_set_pos(w->object,24,24);}
 w->next=app->windows;app->windows=w;app->active=w;
 lv_obj_add_event_cb(w->object,deleted,LV_EVENT_DELETE,w);
 lv_win_add_title(w->object,title?title:"");
 button=lv_win_add_button(w->object,LV_SYMBOL_CLOSE,40);
 lv_obj_add_event_cb(button,close_clicked,LV_EVENT_CLICKED,w);
 header=lv_win_get_header(w->object);
 lv_obj_remove_flag(header,LV_OBJ_FLAG_SCROLLABLE);
 lv_obj_add_flag(header,LV_OBJ_FLAG_CLICKABLE);
 lv_obj_add_event_cb(header,activate,LV_EVENT_PRESSED,w);
 lv_obj_add_event_cb(header,drag,LV_EVENT_PRESSING,w);
 lv_obj_add_event_cb(button,activate,LV_EVENT_PRESSED,w);
 lv_obj_add_event_cb(lv_win_get_content(w->object),activate,LV_EVENT_PRESSED,w);
 return w;
}
lv_obj_t *lg_window_content(lg_window_t *w){return w?lv_win_get_content(w->object):NULL;}
void lg_window_set_geometry(lg_window_t *w,int x,int y,int width,int height){
 if(w&&width>0&&height>0){lv_obj_set_pos(w->object,x,y);lv_obj_set_size(w->object,width,height);}
}
void lg_window_on_close(lg_window_t *w,lg_window_close_fn callback,void *user){if(w){w->close=callback;w->user=user;}}
void lg_window_request_close(lg_window_t *w){
 /* Callback can destroy w. Do not touch it after dispatch. Nested requests
  * are ignored; the DELETE event maintains the app list if it is destroyed. */
 lg_app_t *app;lg_window_t *p;
 if(!w||!w->close||w->requesting)return;
 app=w->app;w->requesting=1;app->dispatching++;w->close(w,w->user);app->dispatching--;
 for(p=app->windows;p;p=p->next)if(p==w){p->requesting=0;break;}
}
void lg_window_destroy(lg_window_t *w){if(w)lv_obj_delete(w->object);}
void lg_app_quit_callback(lg_window_t *w,void *app){lg_app_quit(app?app:(w?w->app:NULL));}
void lg_window_destroy_callback(lg_window_t *w,void *unused){(void)unused;lg_window_destroy(w);}
