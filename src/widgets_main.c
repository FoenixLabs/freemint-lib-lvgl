#include "lvgl_app.h"
#include "demos/widgets/lv_demo_widgets.h"
extern void lg_widgets_physical_keyboard(void);
int main(void){
 lg_app_t *app=lg_app_init("LVGL 9 upstream Widgets demo",600,400,0);int result;
 if(!app)return 1;
 lg_app_on_close(app,lg_app_quit_callback,NULL);
 lv_demo_widgets();lg_widgets_physical_keyboard();
 result=lg_app_run(app);lg_app_close(app);return result;
}
