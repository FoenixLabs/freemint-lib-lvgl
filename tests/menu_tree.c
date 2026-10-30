#include "../src/gem_menu.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int visited[LG_GEM_MENU_OBJECTS];
static void walk(lg_gem_menu *m,int parent){
 int n=m->tree[parent].ob_head,steps=0;
 if(n<0){assert(m->tree[parent].ob_tail==-1);return;}
 for(;;){
  assert(n>0&&n<m->count&&!visited[n]);visited[n]=1;assert(++steps<m->count);
  walk(m,n);
  if(m->tree[n].ob_next==parent){assert(m->tree[parent].ob_tail==n);break;}
  n=m->tree[n].ob_next;
 }
}
static void check(lg_gem_menu *m){
 int i;memset(visited,0,sizeof visited);visited[0]=1;walk(m,0);
 assert(m->count<=LG_GEM_MENU_OBJECTS);
 for(i=0;i<m->count;i++){assert(visited[i]);assert(!!(m->tree[i].ob_flags&OF_LASTOB)==(i==m->count-1));}
}
int main(void){
 lg_menu_model model={0};lg_gem_menu m;int i,found=0;
 strcpy(model.app_name,"Hello LVGL");strcpy(model.menu_title,"LVHELLO");model.count=2;model.item_count=3;
 strcpy(model.titles[0],"File");strcpy(model.titles[1],"Edit");
 model.items[0].menu=1;strcpy(model.items[0].label,"Quit");
 model.items[1].menu=2;strcpy(model.items[1].label,"Clear");
 model.items[2].menu=2;strcpy(model.items[2].label,"-");model.items[2].separator=1;
 assert(lg_gem_menu_build(&m,&model,8,16,1280,1024));check(&m);
 for(i=0;i<m.count;i++)if(m.item_id[i]==2){assert(m.title_id[i]==5);assert(!strcmp(m.tree[i].ob_spec.free_string," Clear "));found++;}
 assert(!strcmp(m.tree[3].ob_spec.free_string," LVHELLO "));
 {char name[LG_MENU_TEXT];lg_gem_menu_name(name,"u:/d/My Apps/My Demo.APP\n");assert(!strcmp(name,"MyDemo"));lg_gem_menu_name(name,"C:\\BIN\\HELLO.APP");assert(!strcmp(name,"HELLO"));lg_gem_menu_name(name,"");assert(!strcmp(name,"LVGL"));}
 assert(found==1);assert(!strcmp(m.registered_name,"  Hello LVGL"));
 assert(!lg_gem_menu_build(&m,&model,8,16,40,1024));
 assert(!lg_gem_menu_build(&m,&model,8,16,1280,80));
 model.count=LG_MENU_MAX;model.item_count=LG_ITEM_MAX;
 for(i=0;i<LG_MENU_MAX;i++)strcpy(model.titles[i],"Menu");
 for(i=0;i<LG_ITEM_MAX;i++){model.items[i].menu=1;model.items[i].separator=0;strcpy(model.items[i].label,"Item");}
 assert(lg_gem_menu_build(&m,&model,8,16,1280,2048));check(&m);
 puts("AES menu tree: links, stable item mapping, copied labels, empty menus, limits and screen bounds passed");return 0;
}
