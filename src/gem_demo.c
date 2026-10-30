#include <gem.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "lvgl.h"
#include "lvgl_gem.h"
#include "lvgl_gem_pixels.h"
#include "lvgl_gem_native.h"
#include "lvgl_gem_wheel.h"
#include "lvgl_gem_render.h"
#include "lvgl_gem_cache.h"

typedef char abi_check[(sizeof(int)==4&&sizeof(short)==2&&sizeof(MFDB)==20)?1:-1];
struct lg_gem {
 short app,win,vdi,opened,mouse_down;unsigned planes;unsigned width,height;size_t stride;
 MFDB backing,screen;void *strip;lv_draw_buf_t native_draw;int direct;lg_scroll_cache *cache;int cache_disabled,lite;
 lv_display_t *display;lv_indev_t *pointer,*keyboard;lv_group_t *group;
 lv_point_t pos;lv_indev_state_t pointer_state,key_state;uint32_t key;
 int running,dirty,captured;lg_rect damage;
 void (*close_request)(void *);void *close_user;
 int measuring;unsigned long convert_ms,copy_ms,converted_pixels,copied_pixels;
};
static uint32_t ticks(void){return (uint32_t)clock()*(1000U/CLOCKS_PER_SEC);}
static void pointer_read(lv_indev_t *in,lv_indev_data_t *data){
 lg_gem *c=lv_indev_get_user_data(in);data->point=c->pos;data->state=c->pointer_state;
}
static void key_read(lv_indev_t *in,lv_indev_data_t *data){
 lg_gem *c=lv_indev_get_user_data(in);data->key=c->key;data->state=c->key_state;
}
static void flush(lv_display_t *display,const lv_area_t *a,uint8_t *pixels){
 lg_gem *c=lv_display_get_user_data(display);uint32_t start=c->measuring?ticks():0;int x,y;unsigned w=lv_area_get_width(a);
 lg_rect r;
 if(lg_intersect((lg_rect){a->x1,a->y1,w,lv_area_get_height(a)},
                 (lg_rect){0,0,c->width,c->height},&r)){
  if(!c->direct)for(y=r.y;y<r.y+r.h;y++){
   uint8_t *row=(uint8_t*)c->backing.fd_addr+y*c->stride;
   const uint8_t *p=pixels+((y-a->y1)*w+r.x-a->x1)*3;
   if(c->planes==1){
    for(x=r.x;x<r.x+r.w;x++,p+=3)lg_pixel888_mono(row,x,y,p[2],p[1],p[0]);
   }else lg_span888_native(row+r.x*(c->planes/8),p,r.w,c->planes);
  }
  if(c->dirty){
   int right=c->damage.x+c->damage.w,bottom=c->damage.y+c->damage.h;
   if(r.x+r.w>right)right=r.x+r.w;
   if(r.y+r.h>bottom)bottom=r.y+r.h;
   if(r.x<c->damage.x)c->damage.x=r.x;
   if(r.y<c->damage.y)c->damage.y=r.y;
   c->damage.w=right-c->damage.x;c->damage.h=bottom-c->damage.y;
  }else c->damage=r;
  c->dirty=1;
 }
 if(c->measuring){c->convert_ms+=ticks()-start;if(!c->direct)c->converted_pixels+=w*lv_area_get_height(a);}
 lv_display_flush_ready(display);
}

static void present(lg_gem *c,lg_rect request){
 short x,y,w,h,clip[4]={0},xy[8],colors[2]={1,0};lg_rect work,r;uint32_t start=c->measuring?ticks():0;
 wind_get(c->win,WF_WORKXYWH,&x,&y,&w,&h);work=(lg_rect){x,y,c->width,c->height};
 wind_update(BEG_UPDATE);wind_get(c->win,WF_FIRSTXYWH,&x,&y,&w,&h);
 while(w>0&&h>0){
  if(lg_intersect((lg_rect){x,y,w,h},work,&r)&&lg_intersect(r,request,&r)){
   if(c->measuring)c->copied_pixels+=(unsigned long)r.w*r.h;
   clip[0]=r.x;clip[1]=r.y;clip[2]=r.x+r.w-1;clip[3]=r.y+r.h-1;
   xy[0]=r.x-work.x;xy[1]=r.y-work.y;xy[2]=xy[0]+r.w-1;xy[3]=xy[1]+r.h-1;
   memcpy(xy+4,clip,sizeof clip);vs_clip(c->vdi,1,clip);v_hide_c(c->vdi);
   if(c->planes==1)vrt_cpyfm(c->vdi,MD_REPLACE,xy,&c->backing,&c->screen,colors);
   else vro_cpyfm(c->vdi,S_ONLY,xy,&c->backing,&c->screen);
   v_show_c(c->vdi,1);
  }
  wind_get(c->win,WF_NEXTXYWH,&x,&y,&w,&h);
 }
 vs_clip(c->vdi,0,clip);wind_update(END_UPDATE);
 if(c->measuring)c->copy_ms+=ticks()-start;
}
static void present_damage(lg_gem *c){
 short x,y,w,h;lg_rect r=c->damage;
 if(!c->dirty)return;
 wind_get(c->win,WF_WORKXYWH,&x,&y,&w,&h);r.x+=x;r.y+=y;
 present(c,r);c->dirty=0;
}
#ifdef LG_WIDGETS_DEMO
/* F8 runs a bounded, repeatable Profile scroll. Timings use MiNTlib's
 * 200 Hz elapsed clock. File I/O is outside the measured region. */
static void benchmark(lg_gem *c){
 lv_obj_t *tabs=lv_obj_get_child(lv_screen_active(),0),*page;
 unsigned i;unsigned long cache_before;uint32_t start,elapsed;FILE *f;char report[256];
 if(!tabs||!lv_obj_check_type(tabs,&lv_tabview_class))return;
 lv_tabview_set_active(tabs,0,LV_ANIM_OFF);
 page=lv_obj_get_child(lv_tabview_get_content(tabs),0);
 lv_obj_scroll_to_y(page,0,LV_ANIM_OFF);lv_refr_now(c->display);
 present(c,(lg_rect){-32768,-32768,65535,65535});c->dirty=0;
 c->convert_ms=c->copy_ms=c->converted_pixels=c->copied_pixels=0;
 cache_before=lg_cache_hits(c->cache);
 c->measuring=1;lg_render_profile(1);start=ticks();
 for(i=0;i<24;i++){
  lv_obj_scroll_to_y(page,(i<12?i+1:23-i)*8,LV_ANIM_OFF);
  lv_refr_now(c->display);
  present_damage(c);
 }
 elapsed=ticks()-start;c->measuring=0;lg_render_profile(0);
 f=fopen("LVBENCH.TXT","a");
 if(f){lv_snprintf(report,sizeof report,"frames=24 elapsed_ms=%lu convert_ms=%lu present_ms=%lu converted_pixels=%lu copied_pixels=%lu depth=%u\n",
  (unsigned long)elapsed,c->convert_ms,c->copy_ms,c->converted_pixels,c->copied_pixels,c->planes);fputs(report,f);
  lv_snprintf(report,sizeof report,"solid_ms=%lu masked_ms=%lu image_ms=%lu fallback_calls=%lu solid_px=%lu masked_px=%lu image_px=%lu\n",lg_render_ms[0],lg_render_ms[1],lg_render_ms[2],lg_render_fallbacks,lg_render_pixels[0],lg_render_pixels[1],lg_render_pixels[2]);fputs(report,f);lv_snprintf(report,sizeof report,"cache_hits=%lu cache_enabled=%d lite=%d\n",lg_cache_hits(c->cache)-cache_before,!c->cache_disabled,c->lite);fputs(report,f);fclose(f);}
}
#endif
void lg_gem_on_close(lg_gem *c,void (*callback)(void *),void *user){c->close_request=callback;c->close_user=user;}
lv_obj_t *lg_gem_root(lg_gem *c){return lv_display_get_screen_active(c->display);}
static void request_close(lg_gem *c){if(c->close_request)c->close_request(c->close_user);}
void lg_gem_close(lg_gem *c){
 if(!c)return;
 lg_cache_delete(c->cache);
 if(c->pointer)lv_indev_delete(c->pointer);
 if(c->keyboard)lv_indev_delete(c->keyboard);
 if(c->display)lv_display_delete(c->display);
 if(c->group){lv_group_set_default(NULL);lv_group_delete(c->group);}
 if(c->win>=0){if(c->opened)wind_close(c->win);wind_delete(c->win);}
 if(c->vdi)v_clsvwk(c->vdi);
 lg_render_native(NULL,0);free(c->strip);free(c->backing.fd_addr);
 if(c->app>=0){lv_deinit();appl_exit();}free(c);
}
lg_gem *lg_gem_open(unsigned width,unsigned height,const char *title){
 lg_gem *c=calloc(1,sizeof(*c));short in[11],out[57],cw,ch,bw,bh,x,y,w,h,ox,oy,ow,oh;
 size_t size;int i;short report[272];unsigned eddi;
 if(!c)return NULL;
 c->app=-1;c->win=-1;
 if(!width||!height||width>4096||height>4096){free(c);return NULL;}
 c->app=appl_init();if(c->app<0){free(c);return NULL;}
 lv_init();lv_tick_set_cb(ticks);c->width=width;c->height=height;
 c->vdi=graf_handle(&cw,&ch,&bw,&bh);for(i=0;i<10;i++)in[i]=1;in[10]=2;
 v_opnvwk(in,&c->vdi,out);if(!c->vdi)goto fail;
 c->planes=lg_vdi_native_format(c->vdi,report,&eddi);
 if(!c->planes){form_alert(1,"[1][Unsupported VDI colour layout.|Need validated RGB565 or XRGB8888.][OK]");goto fail;}
 if(!lg_surface_size(width,height,c->planes,&c->stride,&size))goto fail;
 wind_get(0,WF_WORKXYWH,&x,&y,&w,&h);
 wind_calc(WC_BORDER,NAME|CLOSER|MOVER,0,0,width,height,&ox,&oy,&ow,&oh);
 if(ow>w||oh>h){form_alert(1,"[1][Application needs a larger desktop.|Use at least 640 x 480.][OK]");goto fail;}
 c->direct=c->planes==32;
 c->backing.fd_addr=calloc(1,size);if(!c->direct)c->strip=malloc(width*height*3U);
 if(!c->backing.fd_addr||(!c->direct&&!c->strip))goto fail;
 c->backing.fd_w=width;c->backing.fd_h=height;c->backing.fd_wdwidth=(width+15)/16;c->backing.fd_nplanes=c->planes;
 c->display=lv_display_create(width,height);if(!c->display)goto fail;
 lv_display_set_user_data(c->display,c);
 if(c->direct){
  lv_display_set_color_format(c->display,LV_COLOR_FORMAT_XRGB8888);
  if(lv_draw_buf_init(&c->native_draw,width,height,LV_COLOR_FORMAT_XRGB8888,c->stride,c->backing.fd_addr,size)!=LV_RESULT_OK)goto fail;
  lv_display_set_draw_buffers(c->display,&c->native_draw,NULL);
  lv_display_set_render_mode(c->display,LV_DISPLAY_RENDER_MODE_DIRECT);
  lg_render_native(c->backing.fd_addr,size);
 }else{lv_display_set_color_format(c->display,LV_COLOR_FORMAT_RGB888);
 lv_display_set_buffers(c->display,c->strip,NULL,width*height*3U,LV_DISPLAY_RENDER_MODE_PARTIAL);}
 lv_display_set_flush_cb(c->display,flush);
 c->group=lv_group_create();if(!c->group)goto fail;lv_group_set_default(c->group);
 c->pointer=lv_indev_create();c->keyboard=lv_indev_create();if(!c->pointer||!c->keyboard)goto fail;
 lv_indev_set_type(c->pointer,LV_INDEV_TYPE_POINTER);lv_indev_set_user_data(c->pointer,c);lv_indev_set_read_cb(c->pointer,pointer_read);
 lv_indev_set_type(c->keyboard,LV_INDEV_TYPE_KEYPAD);lv_indev_set_user_data(c->keyboard,c);lv_indev_set_read_cb(c->keyboard,key_read);
 lv_indev_set_group(c->keyboard,c->group);
 c->win=wind_create(NAME|CLOSER|MOVER,x,y,w,h);if(c->win<0)goto fail;
 wind_set_str(c->win,WF_NAME,title);
 /* XaAES WF_WHEEL takes enable, mode (the GEMlib prose omits enable).
  * Request messages only when AES advertises this extension. */
 {short features,b,caps,d;
  if(appl_getinfo(AES_WINDOW,&features,&b,&caps,&d)&&(features&(1<<9)))
   wind_set(c->win,WF_WHEEL,1,WHEEL_MESAG,0,0);
 }
 if(!wind_open(c->win,x+(w-ow)/2,y+(h-oh)/2,ow,oh))goto fail;
 c->opened=1;c->running=1;return c;
fail:lg_gem_close(c);return NULL;
}
#ifdef LG_WIDGETS_DEMO
static void cache_damage(void *user,const lv_area_t *a){lg_gem *c=user;c->damage=(lg_rect){a->x1,a->y1,lv_area_get_width(a),lv_area_get_height(a)};c->dirty=1;}
#endif
int lg_gem_step(lg_gem *c){
 short msg[8],mx,my,buttons,mod,key,clicks,events,x,y,w,h,top,unused;
 uint32_t delay;
 #ifdef LG_WIDGETS_DEMO
 if(c->direct&&!c->cache){lv_obj_t *tabs=lv_obj_get_child(lv_screen_active(),0);
  if(tabs&&lv_obj_check_type(tabs,&lv_tabview_class))c->cache=lg_cache_create(c->display,lv_obj_get_child(lv_tabview_get_content(tabs),0),c->backing.fd_addr,c->stride,cache_damage,c);
 }
 #endif
 delay=lv_timer_handler();
 present_damage(c);
 if(delay>10)delay=10;
 if(delay<1)delay=1;
 events=evnt_multi(MU_MESAG|MU_KEYBD|MU_TIMER|MU_BUTTON,1,1,c->mouse_down?0:1,
  0,0,0,0,0,0,0,0,0,0,msg,delay,&mx,&my,&buttons,&mod,&key,&clicks);
 c->mouse_down=(buttons&1)!=0;
 if(events&MU_MESAG){
  if(msg[0]==AP_TERM)request_close(c);
  else if(msg[3]==c->win)switch(msg[0]){
   case WM_CLOSED:request_close(c);break;
   case WM_TOPPED:wind_set(c->win,WF_TOP,0,0,0,0);break;
   case WM_MOVED:wind_set(c->win,WF_CURRXYWH,msg[4],msg[5],msg[6],msg[7]);break;
   case WM_WHEEL:{
    int horizontal,steps;
    wind_get(c->win,WF_WORKXYWH,&x,&y,&w,&h);
    if(!c->captured&&lg_wheel_decode((uint16_t)msg[7],&horizontal,&steps))
     lg_wheel_scroll(c->display,msg[4]-x,msg[5]-y,horizontal,steps);
    break;
   }
   case WM_REDRAW:present(c,(lg_rect){msg[4],msg[5],msg[6],msg[7]});break;
  }
 }
 wind_get(0,WF_TOP,&top,&unused,&unused,&unused);
 if(!c->running)return 0;
 if(top==c->win){
  if(events&MU_KEYBD){
   unsigned scan=((unsigned short)key)>>8,ascii=key&255;c->key=ascii;
   #ifdef LG_WIDGETS_DEMO
   if(scan==0x41&&!c->lite){extern void lg_widgets_reduce_effects(lv_obj_t *);c->lite=1;lg_widgets_reduce_effects(lv_screen_active());return c->running;} /* F7 */
   if(scan==0x40){c->cache_disabled=!c->cache_disabled;lg_cache_enable(c->cache,!c->cache_disabled);return c->running;} /* F6 */
   if(scan==0x42){benchmark(c);return c->running;} /* F8 */
   #endif
   if(ascii==27)c->key=LV_KEY_ESC;
   else if(ascii==9)c->key=(mod&3)?LV_KEY_PREV:LV_KEY_NEXT;
   else if(ascii==13)c->key=LV_KEY_ENTER;else if(ascii==8)c->key=LV_KEY_BACKSPACE;
   else if(scan==0x4b)c->key=LV_KEY_LEFT;else if(scan==0x4d)c->key=LV_KEY_RIGHT;
   else if(scan==0x48)c->key=LV_KEY_UP;else if(scan==0x50)c->key=LV_KEY_DOWN;
   else if(ascii<32||ascii>126)c->key=0; /* ASCII only */
   if(c->key){c->key_state=LV_INDEV_STATE_PRESSED;lv_indev_read(c->keyboard);c->key_state=LV_INDEV_STATE_RELEASED;lv_indev_read(c->keyboard);}
  }
  /* A button event carries its own ordered mouse snapshot. Resampling here
   * can overwrite a short press with the already-released live state. */
  if(!(events&MU_BUTTON))graf_mkstate(&mx,&my,&buttons,&mod);
  wind_get(c->win,WF_WORKXYWH,&x,&y,&w,&h);
  if(c->captured||((int)mx>=x&&(int)my>=y&&(int)mx<x+(int)c->width&&(int)my<y+(int)c->height)){
   int px=mx-x,py=my-y;
   if(px<0)px=0;
   if(py<0)py=0;
   if(px>=(int)c->width)px=c->width-1;
   if(py>=(int)c->height)py=c->height-1;
   c->pos.x=px;c->pos.y=py;c->pointer_state=(buttons&1)?LV_INDEV_STATE_PRESSED:LV_INDEV_STATE_RELEASED;
   c->captured=(buttons&1)!=0;lv_indev_read(c->pointer);
  }
 }else if(c->captured){c->pointer_state=LV_INDEV_STATE_RELEASED;c->captured=0;lv_indev_read(c->pointer);}
 return c->running;
}
