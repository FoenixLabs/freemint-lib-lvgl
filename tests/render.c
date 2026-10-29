#include "lvgl_gem_render.h"
#include "src/draw/sw/blend/lv_draw_sw_blend_private.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
void lg_base_color(lv_draw_sw_blend_fill_dsc_t *,uint32_t);
void lg_base_image(lv_draw_sw_blend_image_dsc_t *,uint32_t);
void lv_draw_sw_blend_color_to_rgb888(lv_draw_sw_blend_fill_dsc_t *,uint32_t);
void lv_draw_sw_blend_image_to_rgb888(lv_draw_sw_blend_image_dsc_t *,uint32_t);
static void compare(uint8_t *a,uint8_t *b){int x,y;for(y=0;y<2;y++)for(x=0;x<7;x++){uint8_t *p=a+1+y*36+x*4,*q=b+1+y*36+x*4;assert(p[0]==q[3]&&p[1]==q[2]&&p[2]==q[1]);}assert(b[0]==85&&b[35]==85&&b[71]==85);}
void lg_test_render(void){
 uint8_t a[80],b[80],mask[16],src[80];unsigned opa,k,m,cf;
 lv_draw_sw_blend_fill_dsc_t f={0};lv_draw_sw_blend_image_dsc_t i={0};
 for(k=0;k<sizeof mask;k++)mask[k]=k*19;
 for(k=0;k<sizeof src;k++)src[k]=k*37;
 f.dest_w=7;f.dest_h=2;f.dest_stride=36;f.mask_stride=8;f.color=lv_color_make(13,177,239);
 i.dest_w=7;i.dest_h=2;i.dest_stride=36;i.mask_stride=8;i.src_buf=src;i.src_stride=36;i.blend_mode=LV_BLEND_MODE_NORMAL;
 lg_render_native(b,sizeof b);
 for(opa=0;opa<256;opa++)for(m=0;m<2;m++){
  memset(a,85,sizeof a);memset(b,85,sizeof b);f.opa=opa;f.mask_buf=m?mask:NULL;
  f.dest_buf=a+1;lg_base_color(&f,4);f.dest_buf=b+1;lv_draw_sw_blend_color_to_rgb888(&f,4);compare(a,b);
  for(cf=0;cf<4;cf++){
   i.src_color_format=cf==0?LV_COLOR_FORMAT_RGB888:cf==1?LV_COLOR_FORMAT_ARGB8888:cf==2?LV_COLOR_FORMAT_XRGB8888:LV_COLOR_FORMAT_L8;
   memset(a,85,sizeof a);memset(b,85,sizeof b);i.opa=opa;i.mask_buf=m?mask:NULL;
   i.dest_buf=a+1;lg_base_image(&i,4);i.dest_buf=b+1;lv_draw_sw_blend_image_to_rgb888(&i,4);compare(a,b);
  }
 }
 lg_render_native(NULL,0);puts("Native32 fills/images match reference for all opacities/masks, padding and fallback");
}
