#include "wrm/render.h"
#include "stb/stb_image.h"


void test_init(void) 
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
        wrm_fail(1, "Test", "main()", "Failed to start renderer!");
    }
}

int main(int argc, char **argv) {

    test_init();

    wrm_Option_Handle cube = wrm_render_createTestCube();
    if(!cube.exists) {
        wrm_fail(1, "Test", "main()", "failed to create test cubeS!");
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

    if(!wrm_render_setModelTexture(cube.val, bricks_tex.val)) {
        wrm_fail(1, "Test", "main()", "failed to update cube texture!");
    }
    stbi_image_free(td.pixels);
    wrm_render_setModelTransform(cube.val, (vec3){2.0f, 0.0f, -2.0f}, NULL, NULL);

    wrm_Option_Handle triangle = wrm_render_createTestTriangle();
    if(!triangle.exists) wrm_fail(1, "Test", "main()", "failed to create test triangle!");
    wrm_render_setModelTransform(triangle.val, (vec3){2.0f, 0.0f, 2.0f}, NULL, NULL);

    vec3 camera_pos = {-3.0f, 0.0f, 0.0f};
    wrm_render_updateCamera(NULL, NULL, camera_pos, NULL);
    float angle = 0.0f;

    bool should_close = false;

    wrm_render_printDebugData();
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
        vec3 new_rot = {
            [WRM_YAW] = 180.0f, 
            [WRM_PITCH] = 0.0f, 
            [WRM_ROLL] = angle
        };
        
        wrm_render_setModelTransform(triangle.val, NULL, new_rot, NULL);
        wrm_render_draw();
        wrm_render_present();
    }

    wrm_render_quit();
}