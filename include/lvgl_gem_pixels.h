#ifndef LVGL_GEM_PIXELS_H
#define LVGL_GEM_PIXELS_H
#include <stddef.h>
#include <stdint.h>
typedef struct { int x,y,w,h; } lg_rect;
int lg_intersect(lg_rect a, lg_rect b, lg_rect *out);
/* widths bounded to GEM coordinates; planes must be 1, 16 or 32 */
int lg_surface_size(unsigned w,unsigned h,unsigned planes,size_t *stride,size_t *size);
/* Convert native numeric RGB565 to explicit big-endian output bytes.
 * Mono uses MSB-first bits, with one representing black. Preserve other bits. */
void lg_pixel565(uint8_t *row,unsigned x,uint16_t pixel,unsigned planes);
#endif
