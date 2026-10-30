#ifndef LG_PLATFORM_H
#define LG_PLATFORM_H
#include "lvgl.h"
/* Private backend contract. open owns LVGL, display, inputs and clock setup;
 * failure must unwind everything. step pumps one bounded iteration, including
 * timers/presentation, and may invoke close_request. It never decides to quit.
 * A future single-tasking backend can poll devices here without an OS wait.
 * root is the actual surface (native host client or full screen). */
void *lg_platform_open(const char *title, unsigned width, unsigned height,
                       void (*close_request)(void *), void *user);
void lg_platform_step(void *platform);
lv_obj_t *lg_platform_root(void *platform);
void lg_platform_close(void *platform);
const char *lg_platform_error(void);
#endif
