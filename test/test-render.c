#include "wrm/render.h"
#include "stb/stb_image.h"

static wrm_Settings settings = {
    .errors = true,
    .test = true,
    .verbose = true
};

static wrm_Window_Data window_data = {
    .background = 0x100020ffU,
    .height_px = 680,
    .width_px = 800,
    .is_resizable = false,
    .name = "Test wrm-render"
};

int main(int argc, char **argv) {

    if(!wrm_render_init(&settings, &window_data)) {
        wrm_fail(1, "Test", "main()", "Failed to start renderer!");
    }

    wrm_Option_Handle cube = wrm_render_createTestCube();
    if(!cube.exists) {
        wrm_fail(1, "Test", "main()", "failed to create test model!");
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

    if(!wrm_render_updateModelTexture(cube.val, bricks_tex.val)) {
        wrm_fail(1, "Test", "main()", "failed to update cube texture!");
    }
    stbi_image_free(td.pixels);

    wrm_render_updateModelTransform(cube.val, (vec3){4.0f, 0.0f, 0.0f}, NULL, NULL);

    vec3 camera_pos = {-3.0f, 2.0f, -3.0f};
    wrm_render_updateCamera(NULL, NULL, camera_pos, NULL);
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
        // float value = cosf(glm_rad(angle));
        // vec3 new_pos = {value, 0.0f, 0.0f};
        vec3 new_rot = {[WRM_ROLL] = angle, [WRM_PITCH] = 0.0f, [WRM_YAW] = 0.0f};
        
        wrm_render_updateCamera(NULL, NULL, NULL, new_rot);
        // if(!wrm_render_updateModelTransform(cube.val, new_pos, new_rot, NULL)) {
        //     wrm_fail(1, "Test", "main()", "failed to update model transform!");
        // }
        wrm_render_draw();
    }

    wrm_render_quit();
}