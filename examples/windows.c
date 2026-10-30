#include "lvgl_app.h"
static void open_tools(lv_event_t *event)
{
    lg_app_t *app = lv_event_get_user_data(event);
    lg_window_t *tools = lg_window_create(app, "Tools");
    if (!tools) return;
    lg_window_set_geometry(tools, 160, 120, 320, 220);
    lg_window_on_close(tools, lg_window_destroy_callback, NULL);
    lv_obj_t *label = lv_label_create(lg_window_content(tools));
    lv_label_set_text(label, "Close me: the application stays open.\nDrag my title bar to move me.");
    lv_obj_set_width(label, LV_PCT(100));
}
int main(void)
{
    lg_app_t *app = lg_app_init("LVGL application windows");
    if (!app) return 1;
    lg_window_t *main = lg_window_create(app, "Main - close to quit");
    if (!main) { lg_app_close(app); return 1; }
    lg_window_on_close(main, lg_app_quit_callback, app);
    lv_obj_t *button = lv_button_create(lg_window_content(main));
    lv_obj_center(button);
    lv_obj_add_event_cb(button, open_tools, LV_EVENT_CLICKED, app);
    lv_label_set_text(lv_label_create(button), "Open Tools");
    int result = lg_app_run(app);
    lg_app_close(app);
    return result;
}
