#include "lvgl_gem_native.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
static void channel(short *p,unsigned count,unsigned first){unsigned i;for(i=0;i<16;i++)p[i]=i<count?(short)(first+i):-1;}
int main(void){
 short p[272];unsigned depth;
 for(depth=16;depth<=32;depth+=16){
  memset(p,0,sizeof p);p[0]=2;p[1]=2;p[2]=depth;
  p[8]=depth==16?5:8;p[9]=depth==16?6:8;p[10]=depth==16?5:8;
  channel(p+16,p[8],depth==16?11:16);channel(p+32,p[9],depth==16?5:8);channel(p+48,p[10],0);
  assert(lg_vdi_report_format(p,0x100,1,depth)==(int)depth);
  assert(!lg_vdi_report_format(p,0x100,0,depth));
  assert(lg_vdi_report_format(p,0x110,0,depth)==(int)depth);
  assert(!lg_vdi_report_format(p,0x200,1,depth));
  p[14]=128;assert(!lg_vdi_report_format(p,0x110,1,depth));p[14]=0;
  p[16]++;assert(!lg_vdi_report_format(p,0x110,1,depth));p[16]--;
  p[0]=0;assert(!lg_vdi_report_format(p,0x110,1,depth));p[0]=2;
  p[2]=24;assert(!lg_vdi_report_format(p,0x110,1,depth));p[2]=depth;
  p[9]=5;assert(!lg_vdi_report_format(p,0x110,1,depth));
 }
 puts("Native format gate: native RGB565/XRGB8888 accepted; unknown, indexed, swapped and RGB555/24 rejected");
 return 0;
}
