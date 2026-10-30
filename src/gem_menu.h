#ifndef LG_GEM_MENU_H
#define LG_GEM_MENU_H
#include <gem.h>
#include "menu_model.h"
#include "lvgl_gem.h"
#define LG_GEM_MENU_OBJECTS (16+3*LG_MENU_MAX+LG_ITEM_MAX)
typedef struct {
 OBJECT tree[LG_GEM_MENU_OBJECTS];int count;
 int item_id[LG_GEM_MENU_OBJECTS],title_id[LG_GEM_MENU_OBJECTS];
 char labels[LG_GEM_MENU_OBJECTS][LG_MENU_TEXT+4];
 char registered_name[LG_MENU_TEXT+3];int installed;
} lg_gem_menu;
/* Pure tree construction, also tested on host with the real GEM headers. */
void lg_gem_menu_name(char out[LG_MENU_TEXT],const char *path);
int lg_gem_menu_build(lg_gem_menu *out,const lg_menu_model *model,int cw,int ch,int screen_w,int screen_h);
int lg_gem_set_menu(lg_gem *ctx,const lg_menu_model *menu,void (*selected)(void *,int),void *user);
#endif
