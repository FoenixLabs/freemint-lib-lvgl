#ifndef LVGL_APP_H
#define LVGL_APP_H
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif
/* One application/native surface per process; no LVGL window decorations.
 * All calls run on the event-loop thread. The surface belongs to the app.
 * Build with exactly one backend. No native platform types escape this API. */
typedef struct lg_app lg_app_t;
typedef void (*lg_app_close_fn)(lg_app_t *, void *);
enum { LG_APP_RESIZABLE = 1u };
/* Client-area dimensions; flags=0 gives a fixed native window. */
lg_app_t *lg_app_init(const char *title, unsigned width, unsigned height, unsigned flags);
/* Compatibility convenience for a fixed-size surface (flags=0). */
lg_app_t *lg_app_init_size(const char *title, unsigned width, unsigned height);
const char *lg_app_error(void);
int lg_app_run(lg_app_t *app);
void lg_app_quit(lg_app_t *app);
/* Call after run returns, never from an event callback. NULL is allowed. */
void lg_app_close(lg_app_t *app);
lv_obj_t *lg_app_root(lg_app_t *app);
/* Native close requests invoke this handler. With no handler they do nothing.
 * Escape does not quit. Use the helper below or call lg_app_quit explicitly. */
void lg_app_on_close(lg_app_t *app, lg_app_close_fn callback, void *user);
void lg_app_quit_callback(lg_app_t *app, void *unused);
/* Configure menus before run. IDs >0 indicate success; 0 indicates failure.
 * Labels are copied, 1..63 bytes. File and Quit are supplied automatically. */
enum { LG_MENU_FILE = 1, LG_MENU_QUIT = 1 };
int lg_menu_add(lg_app_t *app, const char *title);
int lg_menu_add_item(lg_app_t *app, int menu, const char *label,
                     lg_app_close_fn callback, void *user);
int lg_menu_add_separator(lg_app_t *app, int menu);
/* May also replace the default Quit callback; callable while running. */
int lg_menu_on_item(lg_app_t *app, int item, lg_app_close_fn callback, void *user);
#ifdef __cplusplus
}
#endif
#endif
