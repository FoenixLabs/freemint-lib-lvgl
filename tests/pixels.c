#include "lvgl_gem_pixels.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void){
 unsigned widths[]={1,7,8,15,16,17,319,641};unsigned i,x,p;
 size_t stride,size;lg_rect o;
 uint8_t data[2600];
 for(i=0;i<sizeof(widths)/sizeof(*widths);i++){
  assert(lg_surface_size(widths[i],3,1,&stride,&size));assert(stride==((widths[i]+15)/16)*2);assert(size==stride*3);
  memset(data,0x55,sizeof data);
  for(x=0;x<widths[i];x++)lg_pixel565(data,x,x%2?65535:0,1);
  for(x=0;x<widths[i];x++)assert(((data[x/8]>>(7-x%8))&1)==(x%2==0));
  for(x=widths[i];x<stride*8;x++)assert(((data[x/8]>>(7-x%8))&1)==((0x55>>(7-x%8))&1));
 }
 for(p=0;p<65536;p++){
  lg_pixel565(data,0,p,16);assert(data[0]==p/256&&data[1]==p%256);
  lg_pixel565(data,0,p,32);assert(data[0]==255);
  assert(abs((int)data[1]-(int)(((p/2048)*255+15)/31))<=1);
  assert(abs((int)data[2]-(int)((((p/32)%64)*255+31)/63))<=1);
  assert(abs((int)data[3]-(int)(((p%32)*255+15)/31))<=1);
 }
 assert(!lg_surface_size(0,1,1,&stride,&size));assert(!lg_surface_size(65535,1,32,&stride,&size));
 assert(!lg_surface_size(10,10,24,&stride,&size));
 assert(lg_intersect((lg_rect){10,20,30,40},(lg_rect){0,0,15,25},&o));assert(o.x==10&&o.y==20&&o.w==5&&o.h==5);
 assert(!lg_intersect((lg_rect){0,0,0,1},(lg_rect){0,0,1,1},&o));
 {
  unsigned y;uint8_t expected[4]={0x55,0xaa,0x55,0xaa};
  for(y=0;y<4;y++){
   data[0]=0;
   for(x=0;x<8;x++)lg_pixel888_mono(data,x,y,128,128,128);
   assert(data[0]==expected[y]);
   for(x=0;x<8;x++)lg_pixel888_mono(data,x,y,255,255,255);
   assert(data[0]==0);
   for(x=0;x<8;x++)lg_pixel888_mono(data,x,y,0,0,0);
   assert(data[0]==255);
   lg_pixel888_mono(data,3,y,255,255,255);assert(data[0]==0xef);
  }
 }
 puts("geometry, padding, mono masks and all RGB565 values passed");return 0;
}
