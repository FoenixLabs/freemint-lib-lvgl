#ifndef LVGL_GEM_NATIVE_H
#define LVGL_GEM_NATIVE_H
#include <stdint.h>
/* Returns 1, 16, 32 or 0 (unsupported). Never accesses the screen address. */
int lg_vdi_native_format(short handle,short report[272],unsigned *eddi_version);
/* Pure validation: rejects unknown byte order, RGB555, indexed and packed 24. */
int lg_vdi_report_format(const short *report,unsigned version,int fvdi,unsigned baseline_depth);
void lg_pixel888_native(uint8_t *row,unsigned x,unsigned planes,uint8_t r,uint8_t g,uint8_t b);
#endif
