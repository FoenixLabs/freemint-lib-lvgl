#ifndef LVGL_GEM_BLEND_H
#define LVGL_GEM_BLEND_H
#include <string.h>
#include "src/draw/sw/blend/lv_draw_sw_blend_private.h"
/* v9.2.2's generic masked/opacity RGB888 fill takes bytes from numeric
 * AARRGGBB. On a big-endian CPU those are not B,G,R. Use the upstream custom
 * blend hooks, with explicit channels, without changing dependency sources. */
static inline lv_result_t lg_blend_rgb_fill(lv_draw_sw_blend_fill_dsc_t *d,unsigned size){
 int32_t x,y;uint8_t source[3];
 if(size!=3&&size!=4)return LV_RESULT_INVALID;
 source[0]=d->color.blue;source[1]=d->color.green;source[2]=d->color.red;
 /* Opaque rectangles need no per-pixel opacity/mask decisions. Build the
  * first row, then copy its bytes; preserve destination row padding. */
 if(!d->mask_buf&&d->opa>=LV_OPA_MAX&&d->dest_w>0&&d->dest_h>0){
  uint8_t *first=d->dest_buf,*p=first;
  for(x=0;x<d->dest_w;x++,p+=size){
   p[0]=source[0];p[1]=source[1];p[2]=source[2];if(size==4)p[3]=255;
  }
  for(y=1;y<d->dest_h;y++)memcpy(first+y*d->dest_stride,first,(size_t)d->dest_w*size);
  return LV_RESULT_OK;
 }

 for(y=0;y<d->dest_h;y++){
  uint8_t *row=(uint8_t*)d->dest_buf+y*d->dest_stride;
  const lv_opa_t *mask=d->mask_buf?d->mask_buf+y*d->mask_stride:NULL;
  for(x=0;x<d->dest_w;x++){
   unsigned a=d->opa,k;uint8_t *p=row+x*size;
   if(mask)a=d->opa>=LV_OPA_MAX?mask[x]:LV_OPA_MIX2(d->opa,mask[x]);
   if(!a)continue;
   if(a>=LV_OPA_MAX){p[0]=source[0];p[1]=source[1];p[2]=source[2];}
   else for(k=0;k<3;k++)p[k]=((unsigned)source[k]*a+(unsigned)p[k]*(255-a))>>8;
   if(size==4&&!mask&&d->opa>=LV_OPA_MAX)p[3]=255;
  }
 }
 return LV_RESULT_OK;
}
static inline lv_result_t lg_blend_argb_opaque(lv_draw_sw_blend_fill_dsc_t *d){
 if(d->mask_buf||d->opa<LV_OPA_MAX)return LV_RESULT_INVALID;
 return lg_blend_rgb_fill(d,4);
}
#define LV_DRAW_SW_COLOR_BLEND_TO_RGB888(d,s) lg_blend_rgb_fill(d,s)
#define LV_DRAW_SW_COLOR_BLEND_TO_RGB888_WITH_OPA(d,s) lg_blend_rgb_fill(d,s)
#define LV_DRAW_SW_COLOR_BLEND_TO_RGB888_WITH_MASK(d,s) lg_blend_rgb_fill(d,s)
#define LV_DRAW_SW_COLOR_BLEND_TO_RGB888_MIX_MASK_OPA(d,s) lg_blend_rgb_fill(d,s)
#define LV_DRAW_SW_COLOR_BLEND_TO_ARGB8888(d) lg_blend_argb_opaque(d)
#endif
