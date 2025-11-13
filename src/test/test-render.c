#include "wrm/render.h"
#include "stb/stb_image.h"

int main(int argc, char **argv) {
    wrm_Settings settings = {
        .errors = true,
        .test = true,
        .verbose = true
    };

    wrm_Window_Data window_data = {
        .background = wrm_RGBAf_fromRGBA(0x100020ffU),
        .height_px = WRM_DEFAULT_WINDOW_HEIGHT,
        .width_px = WRM_DEFAULT_WINDOW_WIDTH,
        .is_resizable = false,
        .name = "Test wrm-render"
    };

    if(!wrm_render_init(&settings, &window_data)) {
        wrm_fail(1, "Test", "main()", "Failed to start renderer!");
    }

    wrm_Option_Handle cube = wrm_render_createTestCube();
    if(!cube.exists) {
        wrm_fail(1, "Test", "main()", "failed to create test model!");
    }
    
    wrm_render_updateCamera(-30.0f, 45.0f, 70.0f, 0.0f, (vec3){-3.0f, 2.0f, -3.0f});
    
    

    wrm_Texture_Data td = {.is_font = false };
    int width, height;
    td.pixels = stbi_load("resources/bricks.jpeg", &width, &height, NULL, 4);
    td.height = height;
    td.width = width;
    
    wrm_Option_Handle bricks_tex = wrm_render_createTexture(&td);
    if(!bricks_tex.exists) wrm_fail(1, "Test", "main()", "failed to create bricks texture");

    if(!wrm_render_updateModelTexture(cube.val, bricks_tex.val)) {
        wrm_fail(1, "Test", "main()", "failed to update cube texture!");
    }

    stbi_image_free(td.pixels);

    float angle = 0.0f;

    bool should_close = false;
    
    while(!should_close) {
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) {
                should_close = true;
            }
        }
        angle += 1.0f;
        float value = cosf(glm_rad(angle));
        vec3 new_pos = {value, 0.0f, 0.0f};
        vec3 new_rot = {0.0f, angle, angle};
        
        if(!wrm_render_updateModelTransform(cube.val, new_pos, new_rot, NULL)) {
            wrm_fail(1, "Test", "main()", "failed to update model transform!");
        }
        wrm_render_draw();
    }

    wrm_render_quit();
}