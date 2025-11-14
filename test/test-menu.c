#include "wrm/menu.h"

wrm_Settings settings = {
    .errors = true,
    .test = true,
    .verbose = true
};

wrm_Window_Data window_data = {
    .background = 0x100020ffU,
    .height_px = 680,
    .width_px = 800,
    .is_resizable = false,
    .name = "Test wrm-render"
};

int main(int argc, char **argv)
{
    if(!wrm_render_init(&settings, &window_data)) {
        wrm_fail(1, "Test", "main()", "failed to initialize renderer");
    }
    if(!wrm_menu_init()) {
        wrm_fail(1, "Test", "main()", "failed to initialize the menu system");
    }

    
}