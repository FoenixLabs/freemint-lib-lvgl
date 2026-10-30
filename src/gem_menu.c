#include "gem_menu.h"
#include <string.h>
void lg_gem_menu_name(char out[LG_MENU_TEXT],const char *path){
 const char *base=path,*p,*end;unsigned n=0;
 if(!base)base="";
 for(p=base;*p;p++)if(*p=='/'||*p=='\\'||*p==':')base=p+1;
 end=strrchr(base,'.');if(!end)end=base+strlen(base);
 for(p=base;p<end&&n<LG_MENU_TEXT-1;p++)if((unsigned char)*p>32&&*p!=127)out[n++]=*p;
 out[n]=0;if(!n)strcpy(out,"LVGL");
}
static int object(lg_gem_menu *m,int type,int x,int y,int w,int h){
 int n=m->count++;OBJECT *o=&m->tree[n];
 o->ob_next=o->ob_head=o->ob_tail=-1;o->ob_type=type;
 o->ob_x=x;o->ob_y=y;o->ob_width=w;o->ob_height=h;
 if(type==G_BOX)o->ob_spec.index=0x00011100L; /* black border, white fill */
 return n;
}
static void child(lg_gem_menu *m,int parent,int item){
 OBJECT *p=&m->tree[parent];
 if(p->ob_head<0)p->ob_head=item;else m->tree[p->ob_tail].ob_next=item;
 p->ob_tail=item;m->tree[item].ob_next=parent;
}
static void label(lg_gem_menu *m,int n,const char *s){
 strcpy(m->labels[n]," ");strcat(m->labels[n],s);strcat(m->labels[n]," ");
 m->tree[n].ob_spec.free_string=m->labels[n];
}
int lg_gem_menu_build(lg_gem_menu *m,const lg_menu_model *model,int cw,int ch,int sw,int sh){
 int menus=model->count+1,titles[LG_MENU_MAX+1],x=0,boxes,k,i;
 if(cw<=0||ch<=0||sw<=0||sh<=0||model->count<1||model->count>LG_MENU_MAX||model->item_count<1||model->item_count>LG_ITEM_MAX)return 0;
 memset(m,0,sizeof *m);
 object(m,G_IBOX,0,0,sw,sh);object(m,G_BOX,0,0,sw,ch);object(m,G_IBOX,0,0,sw,ch);
 child(m,0,1);child(m,1,2);
 for(k=0;k<menus;k++){
  const char *s=k?model->titles[k-1]:model->menu_title;int width=(strlen(s)+2)*cw;
  if(x+width>sw)return 0;
  titles[k]=object(m,G_TITLE,x,0,width,ch);label(m,titles[k],s);child(m,2,titles[k]);x+=width;
 }
 m->tree[2].ob_width=x;
 boxes=object(m,G_IBOX,0,ch,sw,sh-ch);child(m,0,boxes);
 for(k=0;k<menus;k++){
  int box,width=0,rows=0,first,y=0,px=m->tree[titles[k]].ob_x;
  if(!k){width=(strlen(model->app_name)+2)*cw;rows=8;}
  else for(i=0;i<model->item_count;i++)if(model->items[i].menu==k){
   int w=(strlen(model->items[i].label)+2)*cw;if(w>width)width=w;rows++;
  }
  if(width<8*cw)width=8*cw;
  if(!rows){rows=1;if(width<9*cw)width=9*cw;}
  if(width>sw||rows*ch>sh-ch)return 0;
  if(px+width>sw)px=sw-width;
  box=object(m,G_BOX,px,0,width,rows*ch);child(m,boxes,box);first=m->count;
  if(!k){
   int n=object(m,G_STRING,0,0,width,ch);label(m,n,model->app_name);m->title_id[n]=titles[k];child(m,box,n);
   /* Standard Desk layout: app/About, separator, six AES accessory slots. */
   for(i=1;i<8;i++){n=object(m,G_STRING,0,i*ch,width,ch);label(m,n,"");if(i==1){memset(m->labels[n],'-',width/cw);m->labels[n][width/cw]=0;}m->tree[n].ob_state=OS_DISABLED;child(m,box,n);}
  }else{
   for(i=0;i<model->item_count;i++)if(model->items[i].menu==k){
    int n=object(m,G_STRING,0,y,width,ch);label(m,n,model->items[i].label);child(m,box,n);y+=ch;
    m->title_id[n]=titles[k];
    if(model->items[i].separator){memset(m->labels[n],'-',width/cw);m->labels[n][width/cw]=0;m->tree[n].ob_state=OS_DISABLED;}
    else m->item_id[n]=i+1;
   }
   if(m->count==first){int n=object(m,G_STRING,0,0,width,ch);label(m,n,"(empty)");m->tree[n].ob_state=OS_DISABLED;child(m,box,n);}
  }
 }
 m->tree[m->count-1].ob_flags|=OF_LASTOB;
 strcpy(m->registered_name,"  ");strcat(m->registered_name,model->app_name);
 return 1;
}
