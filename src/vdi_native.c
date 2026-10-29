#include <gem.h>
#include <gemx.h>
#include <mint/mintbind.h>
#include <mint/ssystem.h>
#include <mint/cookie.h>
#include <string.h>
#include "lvgl_gem_native.h"
/* EdDI cookie function 0 takes its selector in D0, not a C stack argument.
 * Save the target ABI's callee-saved registers around the documented call. */
extern unsigned long lg_eddi_version(long entry);
__asm__(".text\n.even\n.globl _lg_eddi_version\n_lg_eddi_version:\n"
        "move.l 4(%sp),%a0\nmovem.l %d2-%d7/%a2-%a6,-(%sp)\n"
        "moveq #0,%d0\njsr (%a0)\nmovem.l (%sp)+,%d2-%d7/%a2-%a6\n"
        "and.l #65535,%d0\nrts\n");
int lg_vdi_native_format(short handle,short report[272],unsigned *version){
 short baseline[57];long entry=0,fvdi=0;unsigned depth;
 memset(report,0,272*sizeof(*report));*version=0;
 vq_extnd(handle,1,baseline);depth=(unsigned short)baseline[4];
 if(depth==1)return 1;
 if(depth!=16&&depth!=32)return 0;
 /* MiNT's documented cookie query only; no supervisor-mode fallback. */
 if(Ssystem(S_GETCOOKIE,C_EdDI,(long)&entry)<0||!entry)return 0;
 *version=(unsigned)lg_eddi_version(entry);
 if(*version<0x100||*version>=0x200)return 0;
 vq_scrninfo(handle,report);
 if(*version<0x110){
  if(Ssystem(S_GETCOOKIE,0x66564449L,(long)&fvdi)<0)fvdi=0;
 }
 return lg_vdi_report_format(report,*version,fvdi!=0,depth);
}
