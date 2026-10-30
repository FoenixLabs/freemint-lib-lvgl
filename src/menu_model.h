#ifndef LG_MENU_MODEL_H
#define LG_MENU_MODEL_H
#define LG_MENU_MAX 8
#define LG_ITEM_MAX 64
#define LG_MENU_TEXT 64
/* IDs are stable, one-based indices; zero is reserved for the native Desk menu. */
typedef struct { char label[LG_MENU_TEXT];int menu,separator;char shortcut; } lg_menu_item;
typedef struct {
 char menu_title[LG_MENU_TEXT];char app_name[LG_MENU_TEXT];int count,item_count;
 char titles[LG_MENU_MAX][LG_MENU_TEXT];lg_menu_item items[LG_ITEM_MAX];
} lg_menu_model;
#endif
