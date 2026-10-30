#ifndef LVGL_APP_H
#define LVGL_APP_H
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif
/* One application/default surface per process; multiple LVGL windows.
 * All calls run on the event-loop thread. Window handles belong to the app.
 * Build with exactly one backend. No native platform types escape this API. */
typedef struct lg_app lg_app_t;
typedef struct lg_window lg_window_t;
typedef void (*lg_window_close_fn)(lg_window_t *, void *);
typedef void (*lg_app_close_fn)(lg_app_t *, void *);
lg_app_t *lg_app_init(const char *title);
/* Optional explicit preferred surface dimensions (GEM defaults to 600x400). */
lg_app_t *lg_app_init_size(const char *title, unsigned width, unsigned height);
const char *lg_app_error(void);
int lg_app_run(lg_app_t *app);
void lg_app_quit(lg_app_t *app);
/* Call after run returns, never from an event callback. NULL is allowed. */
void lg_app_close(lg_app_t *app);
lv_obj_t *lg_app_root(lg_app_t *app);
/* Used only when no managed window exists, e.g. an upstream full-screen demo. */
void lg_app_on_close(lg_app_t *app, lg_app_close_fn callback, void *user);
lg_window_t *lg_window_create(lg_app_t *app, const char *title);
lv_obj_t *lg_window_content(lg_window_t *window);
void lg_window_set_geometry(lg_window_t *window, int x, int y, int width, int height);
void lg_window_on_close(lg_window_t *window, lg_window_close_fn callback, void *user);
void lg_window_request_close(lg_window_t *window);
void lg_window_destroy(lg_window_t *window);
/* Callback helpers. Close requests without a callback do nothing. */
void lg_app_quit_callback(lg_window_t *window, void *app);
void lg_window_destroy_callback(lg_window_t *window, void *unused);
#ifdef __cplusplus
}
#endif
#endif
