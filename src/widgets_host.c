/* Native desktop preview of the exact same upstream demo; not GEM validation. */
#include <SDL.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lvgl.h"
#include "lvgl_gem_pixels.h"
#include "demos/widgets/lv_demo_widgets.h"
extern void lg_widgets_physical_keyboard(void);
#define WIDTH 600
#define HEIGHT 400
static uint8_t frame[WIDTH*HEIGHT*3],strip[WIDTH*32*3];
static void flush(lv_display_t *d,const lv_area_t *a,uint8_t *p){
 int y;size_t width=lv_area_get_width(a);
 for(y=a->y1;y<=a->y2;y++)memcpy(frame+(y*WIDTH+a->x1)*3,p+(y-a->y1)*width*3,width*3);
 lv_display_flush_ready(d);
}
static lv_point_t pos;static int pressed;
static uint32_t key_value;static lv_indev_state_t key_state;
static void read_key(lv_indev_t *in,lv_indev_data_t *data){(void)in;data->key=key_value;data->state=key_state;}
static void send_key(lv_indev_t *keyboard,uint32_t key){key_value=key;key_state=LV_INDEV_STATE_PRESSED;lv_indev_read(keyboard);key_state=LV_INDEV_STATE_RELEASED;lv_indev_read(keyboard);}
static void read_mouse(lv_indev_t *in,lv_indev_data_t *data){(void)in;data->point=pos;data->state=pressed?LV_INDEV_STATE_PRESSED:LV_INDEV_STATE_RELEASED;}
static lv_obj_t *find_textarea(lv_obj_t *obj){
 uint32_t i;lv_obj_t *found;
 if(lv_obj_check_type(obj,&lv_textarea_class))return obj;
 for(i=0;i<lv_obj_get_child_count(obj);i++){found=find_textarea(lv_obj_get_child(obj,i));if(found)return found;}
 return NULL;
}
static unsigned keyboards(lv_obj_t *obj){
 uint32_t i;unsigned n=lv_obj_check_type(obj,&lv_keyboard_class);
 for(i=0;i<lv_obj_get_child_count(obj);i++)n+=keyboards(lv_obj_get_child(obj,i));return n;
}
static void snapshot(const char *path,int mono){
 unsigned i;uint8_t bits[(WIDTH+15)/16*2*HEIGHT]={0};FILE *f=fopen(path,"wb");if(!f){perror(path);exit(1);}
 if(mono)for(i=0;i<WIDTH*HEIGHT;i++)lg_pixel888_mono(bits+(i/WIDTH)*((WIDTH+15)/16*2),i%WIDTH,i/WIDTH,frame[i*3+2],frame[i*3+1],frame[i*3]);
 fprintf(f,"P6\n%d %d\n255\n",WIDTH,HEIGHT);
 for(i=0;i<WIDTH*HEIGHT;i++){
  if(mono){unsigned byte=(i/WIDTH)*((WIDTH+15)/16*2)+(i%WIDTH)/8;unsigned v=(bits[byte]&(128u>>(i%WIDTH%8)))?0:255;fputc(v,f);fputc(v,f);fputc(v,f);}
  else{fputc(frame[i*3+2],f);fputc(frame[i*3+1],f);fputc(frame[i*3],f);}
 }fclose(f);
}
int main(int argc,char **argv){
 lv_display_t *d;lv_indev_t *mouse,*keyboard;lv_group_t *group;int running=1;
 SDL_Window *window;SDL_Renderer *renderer;SDL_Texture *texture;
 lv_init();d=lv_display_create(WIDTH,HEIGHT);lv_display_set_color_format(d,LV_COLOR_FORMAT_RGB888);
 lv_display_set_buffers(d,strip,NULL,sizeof strip,LV_DISPLAY_RENDER_MODE_PARTIAL);lv_display_set_flush_cb(d,flush);
 group=lv_group_create();lv_group_set_default(group);mouse=lv_indev_create();
 lv_indev_set_type(mouse,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(mouse,read_mouse);
 keyboard=lv_indev_create();lv_indev_set_type(keyboard,LV_INDEV_TYPE_KEYPAD);lv_indev_set_read_cb(keyboard,read_key);lv_indev_set_group(keyboard,group);
 lv_demo_widgets();lg_widgets_physical_keyboard();
 if(argc==3&&(!strcmp(argv[1],"--snapshot")||!strcmp(argv[1],"--smoke-test")||!strcmp(argv[1],"--snapshot-mono"))){
  unsigned i;for(i=0;i<10;i++){lv_tick_inc(10);lv_timer_handler();}lv_refr_now(d);snapshot(argv[2],!strcmp(argv[1],"--snapshot-mono"));
  if(!strcmp(argv[1],"--smoke-test")){
   lv_obj_t *tabs=lv_obj_get_child(lv_screen_active(),0);unsigned tab;
   assert(lv_tabview_get_tab_active(tabs)==0);
   for(tab=1;tab<=2;tab++){
    pos.x=tab*200+100;pos.y=20;pressed=1;lv_indev_read(mouse);
    pressed=0;lv_indev_read(mouse);
    for(i=0;i<40;i++){lv_tick_inc(10);lv_timer_handler();}
    assert(lv_tabview_get_tab_active(tabs)==tab);
   }
   {
    const char *s="A2560Me";lv_obj_t *ta;
    lv_tabview_set_active(tabs,0,LV_ANIM_OFF);lv_obj_update_layout(tabs);
    ta=find_textarea(tabs);assert(ta);assert(keyboards(lv_screen_active())==0);
    lv_group_focus_obj(ta);lv_group_set_editing(group,true);
    for(;*s;s++)send_key(keyboard,(unsigned char)*s);
    assert(!strcmp(lv_textarea_get_text(ta),"A2560Me"));
    lv_obj_send_event(ta,LV_EVENT_DEFOCUSED,NULL);lv_obj_send_event(ta,LV_EVENT_FOCUSED,NULL);
    lv_obj_send_event(ta,LV_EVENT_READY,NULL);lv_obj_send_event(ta,LV_EVENT_CANCEL,NULL);
    lv_obj_update_layout(tabs);assert(lv_obj_get_height(tabs)==HEIGHT);assert(keyboards(lv_screen_active())==0);
   }
   puts("Physical keyboard policy, text input, no virtual keyboard, unchanged viewport passed");
   puts("Upstream Widgets demo: initial render, Analytics/Shop pointer navigation and teardown passed");
  }
  lv_indev_delete(keyboard);lv_indev_delete(mouse);lv_display_delete(d);lv_group_set_default(NULL);lv_group_delete(group);lv_deinit();return 0;
 }
 if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0){fprintf(stderr,"%s\n",SDL_GetError());return 1;}
 window=SDL_CreateWindow("LVGL upstream Widgets demo (host preview)",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WIDTH,HEIGHT,0);
 if(!window)return 1;renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);if(!renderer)return 1;
 texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_BGR24,SDL_TEXTUREACCESS_STREAMING,WIDTH,HEIGHT);if(!texture)return 1;
 lv_tick_set_cb(SDL_GetTicks);SDL_StartTextInput();
 while(running){SDL_Event ev;
  while(SDL_PollEvent(&ev)){
   if(ev.type==SDL_QUIT)running=0;
   if(ev.type==SDL_KEYDOWN&&ev.key.keysym.sym==SDLK_ESCAPE)running=0;
   if(ev.type==SDL_TEXTINPUT){const unsigned char *s=(const unsigned char*)ev.text.text;for(;*s;s++)if(*s>=32&&*s<127)send_key(keyboard,*s);}
   if(ev.type==SDL_KEYDOWN){uint32_t key=0;switch(ev.key.keysym.sym){
    case SDLK_TAB:key=(ev.key.keysym.mod&KMOD_SHIFT)?LV_KEY_PREV:LV_KEY_NEXT;break;
    case SDLK_RETURN:key=LV_KEY_ENTER;break;case SDLK_BACKSPACE:key=LV_KEY_BACKSPACE;break;
    case SDLK_DELETE:key=LV_KEY_DEL;break;case SDLK_LEFT:key=LV_KEY_LEFT;break;
    case SDLK_RIGHT:key=LV_KEY_RIGHT;break;case SDLK_UP:key=LV_KEY_UP;break;case SDLK_DOWN:key=LV_KEY_DOWN;break;
   }if(key)send_key(keyboard,key);}
   if(ev.type==SDL_WINDOWEVENT&&ev.window.event==SDL_WINDOWEVENT_FOCUS_LOST){pressed=0;lv_indev_read(mouse);}
   if(ev.type==SDL_MOUSEMOTION){pos.x=ev.motion.x;pos.y=ev.motion.y;lv_indev_read(mouse);}
   if((ev.type==SDL_MOUSEBUTTONDOWN||ev.type==SDL_MOUSEBUTTONUP)&&ev.button.button==SDL_BUTTON_LEFT){
    pos.x=ev.button.x;pos.y=ev.button.y;pressed=ev.type==SDL_MOUSEBUTTONDOWN;SDL_CaptureMouse(pressed?SDL_TRUE:SDL_FALSE);lv_indev_read(mouse);
   }
  }
  lv_timer_handler();SDL_UpdateTexture(texture,NULL,frame,WIDTH*3);SDL_RenderCopy(renderer,texture,NULL,NULL);SDL_RenderPresent(renderer);SDL_Delay(10);
 }
 lv_indev_delete(keyboard);lv_indev_delete(mouse);lv_display_delete(d);lv_group_set_default(NULL);lv_group_delete(group);lv_deinit();
 SDL_StopTextInput();SDL_DestroyTexture(texture);SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();return 0;
}
