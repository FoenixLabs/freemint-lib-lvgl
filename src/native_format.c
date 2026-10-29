#include "lvgl_gem_native.h"
static int bits_match(const short *p,unsigned count,unsigned first){
 unsigned i;for(i=0;i<count;i++)if(p[i]!=(short)(first+i))return 0;
 for(;i<16;i++)if(p[i]!=-1)return 0;
 return 1;
}
int lg_vdi_report_format(const short *report,unsigned version,int fvdi,unsigned depth){
 if(version<0x100||version>=0x200||(depth!=16&&depth!=32))return 0;
 if(report[0]!=2||report[1]!=2||report[2]!=(short)depth||report[12]!=0)return 0;
 if(version>=0x110){if(report[14]&0x80)return 0;}
 else if(!fvdi)return 0;
 if(depth==16&&report[8]==5&&report[9]==6&&report[10]==5&&report[11]==0&&
    bits_match(report+16,5,11)&&bits_match(report+32,6,5)&&bits_match(report+48,5,0))return 16;
 if(depth==32&&report[8]==8&&report[9]==8&&report[10]==8&&
    (report[11]==0||(report[11]==8&&bits_match(report+64,8,24)))&&
    bits_match(report+16,8,16)&&bits_match(report+32,8,8)&&bits_match(report+48,8,0))return 32;
 return 0;
}
