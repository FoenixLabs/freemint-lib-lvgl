/* GEM raster contract probe. No LVGL or hardware access. */
#include <gem.h>
#include <stddef.h>
#include "probe_log.h"
#include <stdlib.h>
#include <string.h>
#include "lvgl_gem_pixels.h"
typedef char abi_int[(sizeof(int)==4)?1:-1];
typedef char abi_short[(sizeof(short)==2)?1:-1];
typedef char abi_pointer[(sizeof(void*)==4)?1:-1];
typedef char abi_mfdb[(sizeof(MFDB)==20 && offsetof(MFDB,fd_nplanes)==12)?1:-1];
static short win=-1,vhandle=0;
static MFDB bitmap,screen;
static void present(lg_rect request){
 short x,y,w,h,clip[4]={0,0,0,0},xy[8];lg_rect work,visible,r;
 wind_get(win,WF_WORKXYWH,&x,&y,&w,&h);work=(lg_rect){x,y,w,h};
 wind_update(BEG_UPDATE);
 wind_get(win,WF_FIRSTXYWH,&x,&y,&w,&h);
 while(w>0&&h>0){
  visible=(lg_rect){x,y,w,h};
  if(lg_intersect(visible,work,&r)&&lg_intersect(r,request,&r)){
   lg_rect bounds={work.x,work.y,bitmap.fd_w,bitmap.fd_h};
   if(lg_intersect(r,bounds,&r)){
    clip[0]=r.x;clip[1]=r.y;clip[2]=r.x+r.w-1;clip[3]=r.y+r.h-1;
    xy[0]=r.x-work.x;xy[1]=r.y-work.y;xy[2]=xy[0]+r.w-1;xy[3]=xy[1]+r.h-1;
    memcpy(xy+4,clip,sizeof clip);vs_clip(vhandle,1,clip);
    v_hide_c(vhandle);vro_cpyfm(vhandle,S_ONLY,xy,&bitmap,&screen);v_show_c(vhandle,1);
   }
  }
  wind_get(win,WF_NEXTXYWH,&x,&y,&w,&h);
 }
 /* This probe owns its virtual workstation and leaves clipping disabled. */
 vs_clip(vhandle,0,clip);wind_update(END_UPDATE);
}
int main(void){
 short in[11],out[57],ext[57],msg[8],x,y,w,h,cw,ch,bw,bh;
 short mx,my,buttons,keys,key,clicks,ev;int i,done=0,result=1;
 size_t stride,size;unsigned px,py;void *standard=NULL;
 MFDB source;
 if(appl_init()<0)return 1;
 log_handle=Fcreate("LVPROBE.TXT",0);
 vhandle=graf_handle(&cw,&ch,&bw,&bh);
 for(i=0;i<10;i++)in[i]=1;
 in[10]=2;v_opnvwk(in,&vhandle,out);
 if(!vhandle)goto cleanup;
 memset(ext,0,sizeof ext);vq_extnd(vhandle,1,ext);
 log_text("AES=");log_number((unsigned short)aes_global[0],16);log_text("\r\nVDI flag=1 outputs:\r\n");
 for(i=0;i<57;i++){log_number(i,10);log_text("=");log_number((unsigned short)ext[i],16);log_text("\r\n");}
 /* No trusted extension-discovery path yet: do not query EdDI speculatively. */
 if(ext[4]!=1){
  log_text("UNSUPPORTED: Colour needs EdDI discovery and validated packed MFDB contract.\r\n");
  form_alert(1,"[1][Probe supports mono only.|Colour needs validated EdDI/MFDB.][OK]");goto cleanup;
 }
 if(!lg_surface_size(319,193,1,&stride,&size))goto cleanup;
 standard=calloc(1,size);bitmap.fd_addr=calloc(1,size);
 if(!standard||!bitmap.fd_addr)goto cleanup;
 bitmap.fd_w=319;bitmap.fd_h=193;bitmap.fd_wdwidth=stride/2;bitmap.fd_nplanes=1;
 source=bitmap;source.fd_addr=standard;source.fd_stand=1;
 for(py=0;py<193;py++)for(px=0;px<319;px++)
  lg_pixel565((unsigned char*)standard+py*stride,px,((px/16+py/16)&1)?65535:0,1);
 vr_trnfm(vhandle,&source,&bitmap);free(standard);standard=NULL;
 wind_get(0,WF_WORKXYWH,&x,&y,&w,&h);
 win=wind_create(NAME|CLOSER|MOVER, x,y,w,h);
 if(win<0)goto cleanup;
 wind_set_str(win,WF_NAME,"LVGL GEM mono raster probe");
 wind_calc(WC_BORDER,NAME|CLOSER|MOVER,0,0,319,193,&x,&y,&w,&h);
 x=32;y=48;
 if(!wind_open(win,x,y,w,h))goto cleanup;
 while(!done){
  ev=evnt_multi(MU_MESAG|MU_KEYBD,1,1,1,0,0,0,0,0,0,0,0,0,0,msg,0,&mx,&my,&buttons,&keys,&key,&clicks);
  if(ev&MU_KEYBD){if((key&255)==27)done=1;}
  if(ev&MU_MESAG){
   if(msg[0]==AP_TERM)done=1;
   else if(msg[3]==win)switch(msg[0]){
    case WM_CLOSED:done=1;break;
    case WM_TOPPED:wind_set(win,WF_TOP,0,0,0,0);break;
    case WM_MOVED:wind_set(win,WF_CURRXYWH,msg[4],msg[5],msg[6],msg[7]);break;
    case WM_REDRAW:present((lg_rect){msg[4],msg[5],msg[6],msg[7]});break;
   }
  }
 }
 result=0;
cleanup:
 if(win>=0){wind_close(win);wind_delete(win);}
 free(standard);free(bitmap.fd_addr);
 if(vhandle)v_clsvwk(vhandle);
 log_text("Probe exit=");log_number(result,10);log_text("\r\nVisual result must be recorded by operator.\r\n");
 if(log_handle>=0)Fclose((short)log_handle);
 appl_exit();return result;
}
