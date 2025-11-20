#include "render.h"

/*
File-internal types
*/


// data needed to render a model
typedef struct wrm_render_Data {
    mat4 transform;
    wrm_Handle mesh;
    wrm_Handle shader;
    wrm_Handle texture;
    wrm_Handle src_model;
    float distance;
    bool transparent;
} wrm_render_Data;

/*
Constants
*/

const u32 WRM_DEFAULT_WINDOW_HEIGHT = 640;
const u32 WRM_DEFAULT_WINDOW_WIDTH = 800;

const u32 WRM_MESH_TRIANGLE = GL_TRIANGLES;
const u32 WRM_MESH_STRIP = GL_TRIANGLE_STRIP;
const u32 WRM_MESH_FAN = GL_TRIANGLE_FAN;

// general rendering constants

const float WRM_NEAR_CLIP_DISTANCE = 0.001f;
const float WRM_FAR_CLIP_DISTANCE = 1000.0f;

// pool constants

const u32 WRM_RENDER_POOL_INITIAL_CAPACITY = 20;

// list constants

const u32 WRM_RENDER_LIST_INITIAL_CAPACITY = 10;
const u32 WRM_RENDER_LIST_SCALE_FACTOR = 2;

/*
Globals
*/

// Overall module status

bool wrm_render_is_initialized = false;
wrm_render_Settings wrm_render_settings;

// resource pools

wrm_Pool wrm_shaders;
wrm_Pool wrm_meshes;
wrm_Pool wrm_textures;
wrm_Pool wrm_models;


wrm_Tree wrm_model_tree;

bool wrm_show_ui;
bool wrm_render_debug_frame;
u32 wrm_ui_count;


// SDL data 

SDL_Window *wrm_window = NULL; // SDL window 
SDL_GLContext wrm_gl_context; // gl context obtained from SDL

// GL data

wrm_RGBAf wrm_bg_color; // background color
int wrm_window_height;
int wrm_window_width;
vec3 wrm_world_up = {0.0f, 1.0f, 0.0f};

/* a list of models to be drawn (used solely in render_draw() )*/
wrm_Stack wrm_tbd;

/*
Helpers (internal to just this file)
*/
// initializes the internal renderer memory resources
static void wrm_render_initMemory(void);
// sets the GL state before a draw call
static void wrm_render_updateGLState(wrm_render_Data *curr, wrm_render_Data *prev, u32 *count, GLenum *mode, bool *indexed);
// draws a model from the given render data
static void wrm_render_drawModel(wrm_render_Data *draw_data, mat4 view, mat4 persp, u32 count, GLenum mode, bool indexed);
// pack position, rotation, and scale into a transform matrix
static void wrm_render_packTransform(vec3 pos, vec3 rot, vec3 scale, mat4 transform);
// creates a list from the pool of models, sorted by GL state changes
static void wrm_render_prepareModels(void);
// adds a model and its children recursively to the TBD list
static void wrm_render_addModelAndChildren(wrm_Handle m_handle, mat4 parent_transform);
// compares two render data objects for sorting by GL state changes
static int wrm_render_compareRenderData(const void *model1, const void *model2);

// user-visible 

bool wrm_render_init(const wrm_render_Settings *s, const wrm_Window_Data *data)
{
    wrm_render_settings = *s;

    if(SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "ERROR: Render: failed to initialize SDL\n");
        return false;
    }
    if(wrm_render_settings.verbose) printf("Render: initialized SDL\n");

    // set gl attributes: version 3.3 core, with double-buffering
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    u32 sdl_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    if(data->is_resizable) { sdl_flags |= SDL_WINDOW_RESIZABLE; }

    // create the window
    wrm_window = SDL_CreateWindow(
        data->name,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        data->width_px, data->height_px,
        sdl_flags
    );
    SDL_GL_GetDrawableSize(wrm_window, &wrm_window_width, &wrm_window_height);
    if(!wrm_window) {
        fprintf(stderr, "ERROR: Render: Failed to create window\n");
        return false;
    }
    if(wrm_render_settings.verbose) printf("Render: created window\n");

    // get the gl context and load functions
    wrm_gl_context = SDL_GL_CreateContext(wrm_window);
    if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        fprintf(stderr, "ERROR: Render: Failed to initialize GL functions\n");
        return false;
    }
    if(wrm_render_settings.verbose) printf("Render: loaded GL functions\n");

    // setup resource lists
    wrm_render_initMemory();
    if(wrm_render_settings.verbose) printf("Render: created resource pools\n");

    // add default resources to each list: the handle value 0 refers to these
    // setup default shaders
    if(!wrm_render_createDefaultShaders(wrm_render_settings.shaders_dir)) {
        wrm_error("Render", "init()", "unable to create default shaders!");
        return false;
    }
    // setup default texture
    if(!wrm_render_createErrorTexture()) {
        wrm_error("Render", "init()", "unable to create error texture!");
        return false;
    }

    
    if(wrm_render_settings.verbose) printf("Render: created default resources\n");

    // initialize GL data
    wrm_bg_color = wrm_RGBAf_fromRGBA(data->background);
    glViewport(0, 0, wrm_window_width, wrm_window_height);

    // initialize camera with defaults
    wrm_camera = (wrm_Camera){
        .pos = {0.0f, 0.0f, 0.0f},
        .rot = {[WRM_PITCH] = 0.0f, [WRM_YAW] = 0.0f, [WRM_ROLL] = 0.0f},
        .fov = 70.0f,
        .offset = 0.0f,
    };

    wrm_render_is_initialized = true;
    wrm_render_debug_frame = false;
    return true;
}

void wrm_render_quit(void)
{
    if(!wrm_render_is_initialized) return;

    wrm_Pool_delete(&wrm_shaders, wrm_Shader_delete);
    wrm_Pool_delete(&wrm_textures, wrm_Texture_delete);
    wrm_Pool_delete(&wrm_meshes, wrm_Mesh_delete);
    wrm_Pool_delete(&wrm_models, wrm_Model_delete);

    wrm_Stack_delete(&wrm_tbd, NULL);

    SDL_GL_DeleteContext(wrm_gl_context);
    
    if(wrm_window) {
        SDL_DestroyWindow(wrm_window);
        wrm_window = NULL;
    }

    SDL_Quit();
}

void wrm_render_draw(void) 
{
    // handle camera and get view matrix
    mat4 view;
    wrm_render_getViewMatrix(view);

    // get the perspective projection matrix (account for changes in window dimensions and camera fov)
    mat4 persp;
    float aspect_ratio = (float) wrm_window_width / (float) wrm_window_height;
    glm_perspective(wrm_camera.fov, aspect_ratio, WRM_NEAR_CLIP_DISTANCE, WRM_FAR_CLIP_DISTANCE, persp);

    // prepare a list of models for rendering
    wrm_render_prepareModels();
    
    // initialize GL state and tracking of changes
    wrm_render_Data *prev = NULL;
    wrm_render_Data *curr = wrm_tbd.data;
    u32 count = 0;
    GLenum mode = 0;
    bool indexed = false;

    if(wrm_render_debug_frame) {
        printf("\nFRAME DRAW DATA:\n\nMAIN (3D) PASS (%zu model%s to be drawn):\n", wrm_tbd.len, wrm_tbd.len == 1 ? "" : "s");
    }

    // clear the screen
    glClearColor(wrm_bg_color.r, wrm_bg_color.g, wrm_bg_color.b, wrm_bg_color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // render all the opaque models to backbuffer
    for(int i = 0; i < wrm_tbd.len; i++) {
        if(wrm_render_debug_frame) { wrm_render_debugModel(curr->src_model); }

        wrm_render_updateGLState(curr, prev, &count, &mode, &indexed);
        wrm_render_drawModel(curr, view, persp, count, mode, indexed);

        prev = curr;
        curr++;
    }
}

void wrm_render_present(void)
{
    // swap the buffers to present the completed frame
    if(wrm_render_debug_frame) wrm_render_debug_frame = false;
    SDL_GL_SwapWindow(wrm_window);
}

SDL_Window *wrm_render_getWindow(void)
{
    return wrm_window;
}

void wrm_render_onWindowResize(void) 
{
    SDL_GL_GetDrawableSize(wrm_window, &wrm_window_width, &wrm_window_height);
    printf("Window resized to %d %d\n", wrm_window_width, wrm_window_height);
    glViewport(0, 0, wrm_window_width, wrm_window_height);
}

void wrm_render_debugFrame(void)
{
    wrm_render_debug_frame = true;
    printf("DEBUG: Render:\n\nINTERNAL DATA\n");

    printf("\nShaders: %zu total (memory for %zu):\n", wrm_shaders.used, wrm_shaders.cap);
    for(u32 i = 0; i < wrm_shaders.cap; i++) {
        if(wrm_shaders.is_used[i]) {
            wrm_render_debugShader(i);
        }
    }

    printf("\nTextures: %zu total (memory for %zu):\n", wrm_textures.used, wrm_textures.cap);
    for(u32 i = 0; i < wrm_textures.cap; i++) {
        if(wrm_textures.is_used[i]) {
            wrm_render_debugTexture(i);
        }
    }

    printf("\nMeshes: %zu total (memory for %zu):\n", wrm_meshes.used, wrm_meshes.cap);
    for(u32 i = 0; i < wrm_meshes.cap; i++) {
        if(wrm_meshes.is_used[i]) {
            wrm_render_debugMesh(i);
        }
    }

    printf("\nModels: %zu total (memory for %zu):\n", wrm_models.used, wrm_models.cap);
    for(u32 i = 0; i < wrm_models.cap; i++) {
        if(wrm_models.is_used[i]) {
            wrm_render_debugModel(i);
        }
    }

    wrm_render_debugCamera();
}


void wrm_render_getOrientation(const vec3 rot, vec3 forward, vec3 up, vec3 right)
{
    float pitch_r = glm_rad(rot[WRM_PITCH]);
    float yaw_r = glm_rad(rot[WRM_YAW]);
    float roll_r = glm_rad(rot[WRM_ROLL]);

    // get forward from pitch and yaw
    vec3 f = {
        cosf(yaw_r) * cosf(pitch_r),
        sinf(pitch_r),
        -sinf(yaw_r) * cosf(pitch_r)
    };
    glm_vec3_normalize(f);

    vec3 r;
    glm_cross(f, GLM_YUP, r); // cross forward with up to get right (without roll)
    glm_vec3_rotate(r, roll_r, f); // rotate around forward vector to take roll into account
    glm_vec3_normalize(r);

    vec3 u;
    glm_cross(r, f, u); // cross right with facing to get up


    if(forward) glm_vec3_copy(f, forward);
    if(up) glm_vec3_copy(u, up);
    if(right) glm_vec3_copy(r, right);
}

void wrm_render_getOrientationXY(const vec3 rot, vec3 forward, vec3 right)
{
    float yaw_r = glm_rad(wrm_camera.rot[WRM_YAW]);

    vec3 f = {
        cosf(yaw_r),
        0.0f,
        -sinf(yaw_r)
    };
    glm_vec3_normalize(f);
    vec3 r;

    glm_cross(f, GLM_YUP, r); // cross facing with world up to get camera right

    if(forward) glm_vec3_copy(f, forward);
    if(right) glm_vec3_copy(r, right);
}

// module internal 

bool wrm_render_exists(wrm_Handle h, wrm_render_Resource_Type t, const char *caller, const char *context)
{
    bool result;
    char *type;
    switch(t) {
        case WRM_RENDER_RESOURCE_SHADER:
            type = "shader";
            result = h < wrm_shaders.cap && wrm_shaders.is_used[h];
            break;
        case WRM_RENDER_RESOURCE_TEXTURE:
            type = "texture";
            result = h < wrm_textures.cap && wrm_textures.is_used[h];
            break;
        case WRM_RENDER_RESOURCE_MESH:
            type = "mesh";
            result = h < wrm_meshes.cap && wrm_meshes.is_used[h];
            break;
        case WRM_RENDER_RESOURCE_MODEL:
            type = "model";
            result = h < wrm_models.cap && wrm_models.is_used[h];
            break;
        default:
            if(wrm_render_settings.errors) wrm_error("Render", "exists()", "invalid resource type [%d]\n", t);
            return false;
    }

    if(!result && wrm_render_settings.errors) wrm_error("Render", caller, "%s [%u] does not exist %s\n", type, h, context);
    return result;
}

void wrm_render_setGLShader(wrm_Handle shader);
void wrm_render_setGLTexture(wrm_Handle texture);

// helpers

static void wrm_render_initMemory(void)
{
    wrm_Pool_init(&wrm_shaders, WRM_RENDER_POOL_INITIAL_CAPACITY, sizeof(wrm_Shader), true);
    wrm_Pool_init(&wrm_textures, WRM_RENDER_POOL_INITIAL_CAPACITY, sizeof(wrm_Texture), true);
    wrm_Pool_init(&wrm_meshes, WRM_RENDER_POOL_INITIAL_CAPACITY, sizeof(wrm_Mesh), true);
    wrm_Pool_init(&wrm_models, WRM_RENDER_POOL_INITIAL_CAPACITY, sizeof(wrm_Model), true);

    wrm_Stack_init(&wrm_tbd, WRM_RENDER_LIST_INITIAL_CAPACITY, sizeof(wrm_render_Data), true);

    wrm_Tree_init(&wrm_model_tree, &wrm_models, WRM_POOL, offsetof(wrm_Model, tree_node), WRM_MODEL_CHILD_LIMIT);

    wrm_ui_count = 0;
}

static void wrm_render_prepareModels(void)
{
    // clear the list
    wrm_Stack_reset(&wrm_tbd, 0);

    for(u32 i = 0; i < wrm_models.cap; i++) {
        wrm_Model *m = wrm_Pool_at(&wrm_models, i);
        if(m && !m->tree_node.has_parent) {
            wrm_render_addModelAndChildren(i, NULL);
        }
    }

    if(wrm_tbd.len > 1) {
        qsort(wrm_tbd.data, wrm_tbd.len, sizeof(wrm_render_Data), wrm_render_compareRenderData);
    }
}

static void wrm_render_addModelAndChildren(wrm_Handle model, mat4 parent_transform)
{
    wrm_Model *m = wrm_Pool_at(&wrm_models, model);

    wrm_render_Data data; 
    wrm_render_packTransform(m->pos, m->rot, m->scale, data.transform);
    if(parent_transform) {
        glm_mat4_mul(parent_transform, data.transform, data.transform);
    }

    if(m->shown) { 
        data.mesh = m->mesh;
        data.shader = m->shader;
        data.texture = m->texture;
        data.src_model = model;

        wrm_Mesh *mesh = wrm_Pool_at(&wrm_meshes, m->mesh);
        wrm_Texture *texture = wrm_Pool_at(&wrm_textures, m->texture);
        data.transparent = (mesh && mesh->transparent) || (texture && texture->transparent);

        if(data.transparent) {
            data.distance = glm_vec3_distance2(m->pos, wrm_camera.pos); // only care about this if it is transparent
        }
        wrm_Option_Handle top = wrm_Stack_push(&wrm_tbd);
        if(!top.exists) {
            wrm_error("Render", "addModelAndChildren()", "failed to allocate space on draw stack!");
            return;
        }
        memcpy(wrm_Stack_at(&wrm_tbd, top.val), &data, sizeof(data));
    }
    
    if(!(m->tree_node.child_count && m->children_shown)) { return; }

    // add lone child
    if(m->tree_node.child_count == 1) {
        wrm_render_addModelAndChildren(m->tree_node.children, data.transform);
        return;
    }

    // add children list
    u32 *children = wrm_Pool_at(&wrm_model_tree.child_lists, m->tree_node.children);
    for(u8 i = 0; i < m->tree_node.child_count; i++) {
        wrm_render_addModelAndChildren(children[i], data.transform);
    }
}

static void wrm_render_updateGLState(wrm_render_Data *curr, wrm_render_Data *prev, u32 *count, GLenum *mode, bool *indexed)
{
    if(!curr) return;

    if(!prev) {
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    if(prev && curr->transparent && !prev->transparent) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDisable(GL_DEPTH_TEST);
    }
    
    if(!prev || curr->shader != prev->shader) {
        wrm_render_setGLShader(curr->shader);
    }
    
    if(!prev || curr->texture != prev->texture) {
        wrm_render_setGLTexture(curr->texture);
    }

    if(!prev || curr->mesh != prev->mesh) {
        wrm_Mesh *m = (wrm_Mesh*)wrm_meshes.data + curr->mesh;
        glBindVertexArray(m->vao);
        glFrontFace(m->cw ? GL_CW : GL_CCW);
        *count = m->count;
        *mode = m->mode;
        *indexed = m->ebo;
    }
}

void wrm_render_drawModel(wrm_render_Data *draw_data, mat4 view, mat4 persp, u32 count, GLenum mode, bool indexed)
{
    wrm_Shader* shader = wrm_Pool_at(&wrm_shaders, draw_data->shader);

    if(!shader) { return; }

    GLint mvp_loc = glGetUniformLocation(shader->program, "mvp");
    if(mvp_loc != -1) {
        // calculate MVP matrix
        mat4 mvp;
        glm_mat4_copy(draw_data->transform, mvp); // model first
        glm_mat4_mul(view, mvp, mvp);
        glm_mat4_mul(persp, mvp, mvp);
        glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, (float*)mvp);
    }

    if(indexed) {
        glDrawElements(mode, count, GL_UNSIGNED_INT, NULL);
    }
    else {
        glDrawArrays(mode, 0, count);
    }
}

static void wrm_render_packTransform(vec3 pos, vec3 rot, vec3 scale, mat4 transform)
{
    glm_mat4_identity(transform);
    glm_translate(transform, pos);
    
    vec3 as_rad = {glm_rad(rot[0]), glm_rad(rot[1]), glm_rad(rot[2])};
    
    //TODO: need to convert deg->rad, then ensure this is the rotation order I actually want (Y->X->Z, yaw->pitch->roll)
    //versor quat;
    //glm_euler_yxz_quat_rh(as_rad, quat);
    //glm_quat_mat4(quat, transform);

    glm_rotate_y(transform, as_rad[WRM_YAW], transform);
    glm_rotate_z(transform, as_rad[WRM_PITCH], transform);
    glm_rotate_x(transform, as_rad[WRM_ROLL], transform);

    glm_scale(transform, scale);
}

static int wrm_render_compareRenderData(const void *model1, const void *model2)
{
    const wrm_render_Data *m1 = (wrm_render_Data*)model1;
    const wrm_render_Data *m2 = (wrm_render_Data*)model2;

    if(m1->transparent != m2->transparent) {
        return m1->transparent ? 1 : -1; // fully opaque models get rendered first
    }
    if(m1->transparent && m2->transparent) {
        return m1->distance < m2->distance ? 1 : -1; // further back transparent models get rendered first
    }
    if(m1->shader != m2->shader) {
        return (i64)(m1->shader) - (i64)(m2->shader);
    }

    if(m1->texture != m2->texture) {
        return (i64)(m1->texture) - (i64)(m2->texture);
    }

    if(m1->mesh != m2->mesh) {
        return (i64)(m1->mesh) - (i64)(m2->mesh);
    }

    return 0;
}