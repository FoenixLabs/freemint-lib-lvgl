/* Bootstrap template: replace the label with your application's interface. */
#include "lvgl_app.h"
int main(void)
{
    lg_app_t *app = lg_app_init("Hello LVGL", 640, 480, LG_APP_RESIZABLE);
    if (!app) return 1;

    lg_app_on_close(app, lg_app_quit_callback, NULL);

    lv_obj_t *label = lv_label_create(lg_app_root(app));
    lv_label_set_text(label, "Hello from LVGL!");
    lv_obj_center(label);

    int result = lg_app_run(app);
    lg_app_close(app);
    return result;
}
