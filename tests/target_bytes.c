#include "probe_log.h"
#include "lvgl_gem_pixels.h"
#include "lvgl.h"
int main(void){
 unsigned i,j;uint8_t bytes[4];uint16_t values[]={0xf800,0x07e0,0x001f,0xffff};
 log_handle=Fcreate("LVBYTES.TXT",0);if(log_handle<0)return 1;
 for(i=0;i<4;i++){
  lg_pixel565(bytes,0,values[i],16);log_text("565: ");
  for(j=0;j<2;j++){log_number(bytes[j],16);log_text(" ");}log_text("\r\n");
  lg_pixel565(bytes,0,values[i],32);log_text("ARGB: ");
  for(j=0;j<4;j++){log_number(bytes[j],16);log_text(" ");}log_text("\r\n");
 }
 {lv_color_t red=lv_color_make(255,0,0);uint16_t rgb=lv_color_to_u16(red);uint8_t *p=(uint8_t*)&rgb;
 log_text("LVGL red565 (expected f8 0): ");log_number(p[0],16);log_text(" ");log_number(p[1],16);log_text("\r\n");}
 Fclose((short)log_handle);return 0;
}
