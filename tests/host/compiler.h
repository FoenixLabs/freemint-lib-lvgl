/* Host-only calling convention shim for GEMlib's resource-tree declarations.
 * Uses the real GEMlib OBJECT definitions; never used by target builds. */
#ifndef LG_HOST_GEM_COMPILER_H
#define LG_HOST_GEM_COMPILER_H
#include <sys/cdefs.h>
#define __CDECL
#endif
