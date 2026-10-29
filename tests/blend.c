#include "lvgl_gem_blend.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 unsigned size,alpha,x,k;uint8_t storage[64],mask[6]={0,127,255,255,0,64};
 lv_draw_sw_blend_fill_dsc_t d;
 for(size=3;size<=4;size++)for(alpha=0;alpha<256;alpha++){
  memset(storage,85,sizeof storage);memset(&d,0,sizeof d);
  d.dest_buf=storage+4;d.dest_w=3;d.dest_h=2;d.dest_stride=16;
  d.color.red=219;d.color.green=71;d.color.blue=13;d.opa=alpha;
  d.mask_buf=mask;d.mask_stride=3;
  assert(lg_blend_rgb_fill(&d,size)==LV_RESULT_OK);
  for(x=0;x<6;x++){
   unsigned a=alpha>=LV_OPA_MAX?mask[x]:(alpha*mask[x])/256;
   for(k=0;k<3;k++){
    unsigned src=k==0?13:k==1?71:219;
    unsigned expected=a==0?85:a>=LV_OPA_MAX?src:(src*a+85*(255-a))/256;
    assert(storage[4+(x/3)*16+(x%3)*size+k]==expected);
   }
   if(size==4)assert(storage[4+(x/3)*16+(x%3)*size+3]==85);
  }
  assert(storage[3]==85&&storage[63]==85);
 }
 for(size=3;size<=4;size++){
  unsigned y;memset(storage,85,sizeof storage);memset(&d,0,sizeof d);
  d.dest_buf=storage+1;d.dest_w=3;d.dest_h=3;d.dest_stride=17;d.opa=255;
  d.color.red=219;d.color.green=71;d.color.blue=13;
  assert(lg_blend_rgb_fill(&d,size)==LV_RESULT_OK);
  for(y=0;y<3;y++){
   for(x=0;x<3;x++){
    uint8_t *p=storage+1+y*17+x*size;
    assert(p[0]==13&&p[1]==71&&p[2]==219);if(size==4)assert(p[3]==255);
   }
   for(x=3*size;x<17;x++)assert(storage[1+y*17+x]==85);
  }
  assert(storage[0]==85&&storage[63]==85);
 }
 memset(&d,0,sizeof d);d.dest_buf=storage;d.dest_w=1;d.dest_h=1;d.dest_stride=4;d.opa=255;
 d.color.red=255;assert(lg_blend_argb_opaque(&d)==LV_RESULT_OK);
 assert(storage[0]==0&&storage[1]==0&&storage[2]==255&&storage[3]==255);
 puts("Portable blend: all opacities, masks, channel order, alpha and padded rows passed");return 0;
}
