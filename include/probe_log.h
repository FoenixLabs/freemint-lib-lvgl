#ifndef PROBE_LOG_H
#define PROBE_LOG_H
/* Integer-only logging avoids pulling floating point printf from external libc. */
#include <mint/osbind.h>
#include <string.h>
static long log_handle=-1;
static void log_text(const char *s){if(log_handle>=0)Fwrite((short)log_handle,(long)strlen(s),s);}
static void log_number(unsigned long n,unsigned base){
 char b[33];unsigned i=32;b[i]=0;
 do{b[--i]="0123456789abcdef"[n%base];n/=base;}while(n&&i);
 log_text(b+i);
}
#endif
