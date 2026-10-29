#include "lvgl_gem_render.h"
#include "src/draw/sw/blend/lv_draw_sw_blend_private.h"
#include <stdint.h>
#include <string.h>
#include <time.h>
/* The build renames only the two original definitions, not their callers.
 * This adapter delegates all non-native destinations to the pinned renderer. */
void lg_base_color(lv_draw_sw_blend_fill_dsc_t *,uint32_t);
void lg_base_image(lv_draw_sw_blend_image_dsc_t *,uint32_t);
static uintptr_t first,last;static int profiling;
unsigned long lg_render_ms[3],lg_render_pixels[3],lg_render_fallbacks;
static unsigned long now(void){return (unsigned long)clock()*(1000U/CLOCKS_PER_SEC);}
void lg_render_native(void *buffer,size_t bytes){first=(uintptr_t)buffer;last=first+bytes;}
void lg_render_profile(int enabled){profiling=enabled;if(enabled){memset(lg_render_ms,0,sizeof lg_render_ms);memset(lg_render_pixels,0,sizeof lg_render_pixels);lg_render_fallbacks=0;}}
static int native(void *p,uint32_t size){return size==4&&first&&(uintptr_t)p>=first&&(uintptr_t)p<last;}
static void mix(uint8_t *p,unsigned r,unsigned g,unsigned b,unsigned a){
 if(!a)return;
 if(a>=LV_OPA_MAX){p[1]=r;p[2]=g;p[3]=b;}
 else{unsigned inv=255-a;p[1]=(r*a+p[1]*inv)>>8;p[2]=(g*a+p[2]*inv)>>8;p[3]=(b*a+p[3]*inv)>>8;}
 p[0]=255;
}
void lv_draw_sw_blend_color_to_rgb888(lv_draw_sw_blend_fill_dsc_t *d,uint32_t size){
 unsigned bucket=d->mask_buf?1:0;unsigned long start=profiling?now():0;int x,y;
 if(!native(d->dest_buf,size))lg_base_color(d,size);
 else if(!d->mask_buf&&d->opa>=LV_OPA_MAX){
  uint8_t *row=d->dest_buf,*p=row;
  for(x=0;x<d->dest_w;x++,p+=4){p[0]=255;p[1]=d->color.red;p[2]=d->color.green;p[3]=d->color.blue;}
  for(y=1;y<d->dest_h;y++)memcpy(row+y*d->dest_stride,row,d->dest_w*4U);
 }else{
  for(y=0;y<d->dest_h;y++){
   uint8_t *p=(uint8_t*)d->dest_buf+y*d->dest_stride;
   const uint8_t *m=d->mask_buf?d->mask_buf+y*d->mask_stride:NULL;
   for(x=0;x<d->dest_w;x++,p+=4){unsigned a=m?(d->opa>=LV_OPA_MAX?m[x]:LV_OPA_MIX2(d->opa,m[x])):d->opa;mix(p,d->color.red,d->color.green,d->color.blue,a);}
  }
 }
 if(profiling){lg_render_ms[bucket]+=now()-start;lg_render_pixels[bucket]+=(unsigned long)d->dest_w*d->dest_h;}
}
/* Lossless temporary byte reversal lets uncommon formats/blend modes use the
 * original renderer. It touches only this clipped destination, never the screen. */
static void reverse(uint8_t *p,int w,int h,int stride){int x,y;for(y=0;y<h;y++,p+=stride)for(x=0;x<w;x++){uint8_t *q=p+x*4,a=q[0],b=q[1];q[0]=q[3];q[1]=q[2];q[2]=b;q[3]=a;}}
void lv_draw_sw_blend_image_to_rgb888(lv_draw_sw_blend_image_dsc_t *d,uint32_t size){
 unsigned long start=profiling?now():0;int x,y;
 if(!native(d->dest_buf,size))lg_base_image(d,size);
 else if(d->blend_mode!=LV_BLEND_MODE_NORMAL ||
  (d->src_color_format!=LV_COLOR_FORMAT_RGB888&&d->src_color_format!=LV_COLOR_FORMAT_XRGB8888&&d->src_color_format!=LV_COLOR_FORMAT_ARGB8888)){
  reverse(d->dest_buf,d->dest_w,d->dest_h,d->dest_stride);lg_base_image(d,size);
  reverse(d->dest_buf,d->dest_w,d->dest_h,d->dest_stride);if(profiling)lg_render_fallbacks++;
 }else{
  unsigned step=d->src_color_format==LV_COLOR_FORMAT_RGB888?3:4;
  int alpha=d->src_color_format==LV_COLOR_FORMAT_ARGB8888;
  for(y=0;y<d->dest_h;y++){
   uint8_t *p=(uint8_t*)d->dest_buf+y*d->dest_stride;
   const uint8_t *s=(const uint8_t*)d->src_buf+y*d->src_stride,*m=d->mask_buf?d->mask_buf+y*d->mask_stride:NULL;
   for(x=0;x<d->dest_w;x++,p+=4,s+=step){
    unsigned a;
    if(alpha)a=m?(d->opa>=LV_OPA_MAX?LV_OPA_MIX2(s[3],m[x]):LV_OPA_MIX3(s[3],m[x],d->opa)):(d->opa>=LV_OPA_MAX?s[3]:LV_OPA_MIX2(s[3],d->opa));
    else a=m?(d->opa>=LV_OPA_MAX?m[x]:LV_OPA_MIX2(m[x],d->opa)):d->opa;
    mix(p,s[2],s[1],s[0],a);
   }
  }
 }
 if(profiling){lg_render_ms[2]+=now()-start;lg_render_pixels[2]+=(unsigned long)d->dest_w*d->dest_h;}
}
