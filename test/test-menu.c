#include "wrm/render.h"
#include "stb/stb_image.h"
#include "wrm/gui.h"



int main(int argc, char **argv)
{
    wrm_render_Settings settings = {
        .errors = true,
        .test = true,
        .verbose = true,
        .shaders_dir = "src/shaders"
    };

    wrm_Window_Data window_data = {
        .background = 0x100020ffU,
        .height_px = 680,
        .width_px = 800,
        .is_resizable = false,
        .name = "Test wrm-render"
    };

    if(!wrm_render_init(&settings, &window_data)) {
        wrm_fail(1, "Test", "main()", "failed to initialize renderer");
    }

    if(!wrm_gui_init("src/shaders")) {
        wrm_fail(1, "Test", "main()", "failed to initialize the menu system");
    }

    // texture test
    int width, height;
    wrm_Texture_Data td = {
        .pixels = stbi_load("resources/bricks.jpeg", &width, &height, NULL, 4)
    };
    td.height = height;
    td.width = width;
    
    wrm_Option_Handle bricks_tex = wrm_render_createTexture(&td);
    if(!bricks_tex.exists) wrm_fail(1, "Test", "main()", "failed to create bricks texture");

    wrm_gui_Properties p = { .visible = true };
    wrm_Option_Handle image = wrm_gui_createImage(p, bricks_tex.val);

    if(!image.exists) wrm_fail(1, "Test", "main()", "failed to create image element");


    bool should_close = false;
    u32 i = 0;
    while(!should_close) {
        
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) {
                should_close = true;
            }
        }

        if(i % 60 == 0) {
            wrm_gui_Alignment a = (wrm_gui_Alignment) {
                .x = rand() % 200, .y = rand() % 170, .width = 100, .height = 100,
                .x_from = rand() % 3, .x_is = rand() % 3,
                .y_from = rand() % 3 + 2, .y_is = rand() % 3 + 2
            };

            wrm_gui_setAlignment(image.val, a);

            wrm_gui_debugAlignment(a);
            printf("\n");

            wrm_render_printDebugData();
        }
        
        i++;

        wrm_render_draw();
        wrm_gui_draw();
        wrm_render_present();
    }
    
    wrm_gui_quit();
    wrm_render_quit();
}