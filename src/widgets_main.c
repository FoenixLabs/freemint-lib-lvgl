#include "lvgl_gem.h"
#include "demos/widgets/lv_demo_widgets.h"
extern void lg_widgets_physical_keyboard(void);
int main(void){
 lg_gem *window=lg_gem_open(600,400,"LVGL 9 upstream Widgets demo");
 if(!window)return 1;
 lv_demo_widgets();
 lg_widgets_physical_keyboard();
 while(lg_gem_step(window)){}
 lg_gem_close(window);return 0;
}
