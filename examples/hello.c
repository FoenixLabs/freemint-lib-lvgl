/* Bootstrap template: replace the label with your application's interface. */
#include "lvgl_app.h"
int main(void)
{
    lg_app_t *app = lg_app_init("Hello LVGL");
    if (!app) return 1;

    lg_window_t *window = lg_window_create(app, "Main");
    if (!window) { lg_app_close(app); return 1; }
    lg_window_on_close(window, lg_app_quit_callback, app);

    lv_obj_t *label = lv_label_create(lg_window_content(window));
    lv_label_set_text(label, "Hello from LVGL!");
    lv_obj_center(label);

    int result = lg_app_run(app);
    lg_app_close(app);
    return result;
}
