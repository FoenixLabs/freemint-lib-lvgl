#ifndef LVGL_GEM_WHEEL_H
#define LVGL_GEM_WHEEL_H
#include "lvgl.h"
/* WM_WHEEL orientation is authoritative (includes AES reverse-wheel policy).
 * XaAES packs a signed 8-bit count; normalize its magnitude before direction. */
int lg_wheel_decode(uint16_t info,int *horizontal,int *steps);
/* Client coordinates; scroll nearest eligible hovered container without clicks
 * or focus changes. Positive steps move down/right; 32 pixels per notch. */
int lg_wheel_scroll(lv_display_t *display,int x,int y,int horizontal,int steps);
#endif
