#include "lvgl_app.h"
#include "platform.h"
#include <stdlib.h>
#include <string.h>
struct lg_app {
 void *platform;int quit,running,dispatching,started;
 lg_menu_model menu;
 lg_app_close_fn actions[LG_ITEM_MAX];void *action_users[LG_ITEM_MAX];
 lg_app_close_fn close;void *user;
};
static lg_app_t *owner;
static const char *error;
static void request_close(void *user){
 lg_app_t *app=user;
 if(!app->close||app->dispatching)return;
 app->dispatching=1;app->close(app,app->user);app->dispatching=0;
}
lg_app_t *lg_app_init_size(const char *title,unsigned width,unsigned height){return lg_app_init(title,width,height,0);}
lg_app_t *lg_app_init(const char *title,unsigned width,unsigned height,unsigned flags){
 lg_app_t *app;error=NULL;
 if(owner){error="An application is already initialized";return NULL;}
 if(flags&~LG_APP_RESIZABLE){error="Unknown application flags";return NULL;}
 if((flags&LG_APP_RESIZABLE)&&(width<64||height<48)){error="Resizable surfaces need at least 64x48 pixels";return NULL;}
 if(!width||!height){error="Surface dimensions must be positive";return NULL;}
 app=calloc(1,sizeof *app);if(!app){error="Cannot allocate application";return NULL;}
 strncpy(app->menu.app_name,title?title:"LVGL",LG_MENU_TEXT-1);
 app->menu.count=1;strcpy(app->menu.titles[0],"File");
 app->menu.item_count=1;app->menu.items[0].menu=LG_MENU_FILE;strcpy(app->menu.items[0].label,"Quit");
 app->actions[0]=lg_app_quit_callback;app->menu.items[0].shortcut='Q';
 owner=app; /* Also rejects reentrant initialization during backend setup. */
 app->platform=lg_platform_open(title?title:"LVGL",width,height,(flags&LG_APP_RESIZABLE)!=0,request_close,app);
 if(!app->platform){error=lg_platform_error();free(app);owner=NULL;return NULL;}
 return app;
}
const char *lg_app_error(void){return error?error:"";}
static void menu_selected(void *user,int item){
 lg_app_t *app=user;
 if(item<=0||item>app->menu.item_count||app->menu.items[item-1].separator)return;
 if(app->actions[item-1]){app->dispatching++;app->actions[item-1](app,app->action_users[item-1]);app->dispatching--;}
}
int lg_app_run(lg_app_t *app){
 if(!app||app->running)return -1;
 if(!app->started&&!app->quit){
  if(!lg_platform_menu(app->platform,&app->menu,menu_selected,app)){error="Cannot install native menu (layout or allocation)";return -1;}
  app->started=1;
 }
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

static int label_valid(const char *s){
 size_t n;if(!s||!(n=strlen(s))||n>=LG_MENU_TEXT)return 0;
 for(;*s;s++)if((unsigned char)*s<32||*s==127)return 0;
 return 1;
}
int lg_menu_add(lg_app_t *app,const char *title){
 if(!app||app->started||app->menu.count==LG_MENU_MAX||!label_valid(title)){error="Cannot add menu: invalid label, limit or event loop already started";return 0;}
 strcpy(app->menu.titles[app->menu.count],title);return ++app->menu.count;
}
int lg_menu_add_item(lg_app_t *app,int menu,const char *label,lg_app_close_fn callback,void *user){
 int i;
 if(!app||app->started||menu<1||menu>app->menu.count||app->menu.item_count==LG_ITEM_MAX||!label_valid(label)){error="Cannot add item: invalid menu/label, limit or event loop already started";return 0;}
 i=app->menu.item_count++;app->menu.items[i].menu=menu;strcpy(app->menu.items[i].label,label);
 app->actions[i]=callback;app->action_users[i]=user;return i+1;
}
int lg_menu_add_separator(lg_app_t *app,int menu){
 int item=lg_menu_add_item(app,menu,"-",NULL,NULL);
 if(item)app->menu.items[item-1].separator=1;
 return item;
}
int lg_menu_on_item(lg_app_t *app,int item,lg_app_close_fn callback,void *user){
 if(!app||item<1||item>app->menu.item_count||app->menu.items[item-1].separator)return 0;
 app->actions[item-1]=callback;app->action_users[item-1]=user;return 1;
}

int lg_menu_set_shortcut(lg_app_t *app,int item,const char *shortcut){
 int i;char key=0;
 if(!app||app->started||item<1||item>app->menu.item_count||app->menu.items[item-1].separator){error="Cannot set shortcut: invalid item or event loop already started";return 0;}
 if(shortcut&&*shortcut){
  if(shortcut[0]!='^'||!shortcut[1]||shortcut[2]){error="Shortcut must be ^A through ^Z";return 0;}
  key=shortcut[1];if(key>='a'&&key<='z')key-=32;
  if(key<'A'||key>'Z'){error="Shortcut must be ^A through ^Z";return 0;}
 }
 for(i=0;key&&i<app->menu.item_count;i++)if(i!=item-1&&app->menu.items[i].shortcut==key){error="Shortcut is already assigned";return 0;}
 app->menu.items[item-1].shortcut=key;return 1;
}
