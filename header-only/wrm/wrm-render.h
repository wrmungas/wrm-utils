#ifndef WRM_RENDER_H
#define WRM_RENDER_H

/*
File wrm-render.h

Created Oct 29, 2025 
by William R Mungas (wrm)

Version: 0.1.0 
(Last modified Nov 6, 2025)

DESCRIPTION:
Rendering framework that uses SDL to create a window, which is exposed externally 
for wrm-input. Currently uses cglm as a vector/matrix gl-optimized math library

In the future, I may abstract general window functionality to a separate header called
wrm-window or something similar, so that this and wrm-input are less tightly coupled. 
I may also provide a rendering backend based on Vulkan instead of OpenGL.

PROVIDES:
- rendering primitives: shader, texture, mesh, model
- access to a default camera

REQUIREMENTS:
- SDL2 must be installed for headers/linking
- cglm/ must also be in your project's includes, used as header-only lib
*/

#include "wrm/common.h"
#include "SDL2/SDL.h"
#include "cglm/cglm.h"


/*
Type Declarations
*/

// window creation arguments
typedef struct wrm_Window_Data wrm_Window_Data; 

// shader-related
typedef struct wrm_Shader_Defaults wrm_Shader_Defaults;

// Color and texture related types

// single 32-bit integer rgba value
typedef u32 wrm_RGBA;
// collection of 4 bytes: r, g, b, a
typedef struct wrm_RGBAi wrm_RGBAi;
// collection of 4 floats: r, g, b, a: used for when OpenGL wants color channel values as floats in the range [0.0f, 1.0f]
typedef struct wrm_RGBAf wrm_RGBAf;
// data for creating a texture
typedef struct wrm_Texture_Data wrm_Texture_Data;

// Mesh types

// Arguments for mesh creation
typedef struct wrm_Mesh_Data wrm_Mesh_Data;
// Enum of Mesh types
typedef enum wrm_Mesh_Type wrm_Mesh_Type;

// model types

// Arguments for model creation
typedef struct wrm_Model_Data wrm_Model_Data;

/*
Externally visible constants
*/

extern const u32 WRM_MESH_TRIANGLE;
extern const u32 WRM_MESH_STRIP;
extern const u32 WRM_MESH_FAN;

extern const u32 WRM_DEFAULT_WINDOW_WIDTH;
extern const u32 WRM_DEFAULT_WINDOW_HEIGHT;

#define WRM_MODEL_CHILD_LIMIT 4

// bit masks for colors 

#define WRM_RGBA_R_BITS 0xff000000u
#define WRM_RGBA_G_BITS 0x00ff0000u
#define WRM_RGBA_B_BITS 0x0000ff00u
#define WRM_RGBA_A_BITS 0x000000ffu

// a few simple predefined colors

#define WRM_RGBA_BLACK 0x000000ffu
#define WRM_RGBA_WHITE 0xffffffffu

#define WRM_RGBA_RED 0xff0000ffu
#define WRM_RGBA_GREEN 0x00ff00ffu
#define WRM_RGBA_BLUE 0x0000ffffu

#define WRM_RGBA_PURPLE 0xff00ffffu
#define WRM_RGBA_YELLOW 0xffff00ffu
#define WRM_RGBA_CYAN 0x00ffffffu

// Externally visible type definitions

struct wrm_Shader_Defaults {
    wrm_Handle color;
    wrm_Handle texture;
    wrm_Handle both;
};

struct wrm_RGBAi {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};

struct wrm_RGBAf {
    float r;
    float g;
    float b;
    float a;
};

struct wrm_Window_Data {
    const char *name;
    i32 height_px;
    i32 width_px;
    bool is_resizable;
    wrm_RGBAf background;
};

struct wrm_Texture_Data {
    u8 *pixels; // must be components * width * height long
    u32 width;
    u32 height;
    bool font;
};

struct wrm_Mesh_Data {
    float *positions;       // position for each vertex
    float *colors;          // RGBA color for each vertex
    float *uvs;             // uv for each vertex
    u32 *indices;           // vertex indices
    size_t vtx_cnt;         // number of vertices for which we have data (independent of number of triangles)
    size_t idx_cnt;         // the number of indices in the mesh (3 * total tris for GL_TRIANGLES)
    u32 mode;
    bool cw;                // clockwise winding order?
    bool dynamic;           // will we be frequently updating this mesh?
    bool ui;                // is this a ui mesh? (all positions are x-y in orthographic view space, assumed to be the same z)
};

struct wrm_Model_Data { 
    // these are relative to the parent model
    vec3 pos;
    vec3 rot;
    vec3 scale;

    wrm_Handle mesh;
    wrm_Handle texture; // only used when the model has a textured mesh; for now, meshes only use a single texture
    wrm_Handle shader;
    bool is_visible;
    bool is_ui; // whether to draw as part of the UI or not
};


// Externally visible values

extern wrm_Shader_Defaults wrm_shader_defaults;

// Externally visible functions

/*
Initialize the renderer - opens a window with a GL context, sets up default shaders 
*/
bool wrm_render_init(const wrm_Settings *settings, const wrm_Window_Data *data);

/*
Shut down the renderer and clean up resources
*/
void wrm_render_quit(void);

/*
Cleans up any unused/freed resources and renders all currently visible objects
*/
void wrm_render_draw(float delta_time);


/*
Get the window created by the render
May be needed by other modules
*/
SDL_Window *wrm_render_getWindow(void);

/*
Updates renderer on a window resize event
*/
void wrm_render_onWindowResize(void);
/* Prints debug info about current render state to standard output */
void wrm_render_printDebugData(void);
/* For debugging; prints a shader's data to `stdout` */
void wrm_render_printShaderData(wrm_Handle shader);
/* For debugging; prints a stexture's data to `stdout` */
void wrm_render_printTextureData(wrm_Handle texture);
/* For debugging; prints a mesh's data to `stdout` */
void wrm_render_printMeshData(wrm_Handle mesh);
/* For debugging; prints a model's data to `stdout` */
void wrm_render_printModelData(wrm_Handle model);

/* Set a flag for whether or not to show the GUI elements */
void wrm_render_setUIShown(bool show_ui);

/*
Creates a shader program using the given frag and vert
*/
wrm_Option_Handle wrm_render_createShader(const char *vert, const char *frag, bool needs_col, bool needs_tex);

/*
Create a texture
*/
wrm_Option_Handle wrm_render_createTexture(const wrm_Texture_Data *data);

/*
Update part of a texture from the given x and y offset with the given data
*/
bool wrm_render_updateTexture(wrm_Handle texture, wrm_Texture_Data *data, u32 x, u32 y);

/* Create a mesh */
wrm_Option_Handle wrm_render_createMesh(const wrm_Mesh_Data *data);
/* Clones an existing mesh (useful for changing data without affecting all instances of this mesh)*/
wrm_Option_Handle wrm_render_cloneMesh(wrm_Handle mesh);
/* Updates a mesh's data: IMPORTANT: will update ALL existing instances of this mesh */
bool wrm_render_updateMesh(wrm_Handle mesh, const wrm_Mesh_Data *data);

// model-related

/* Create a model - if use_default_shader is true, the renderer will attempt to select a default shader based on the mesh attributes */
wrm_Option_Handle wrm_render_createModel(const wrm_Model_Data *data, wrm_Handle parent, bool use_default_shader);
/* Update a model's transform data */
void wrm_render_updateModelTransform(wrm_Handle model, const vec3 pos, const vec3 rot, const vec3 scale);
/* Update a model's mesh */
void wrm_render_updateModelMesh(wrm_Handle model, wrm_Handle mesh);
/* Update a model's texture*/
void wrm_render_updateModelTexture(wrm_Handle model, wrm_Handle texture);
/* Update a model's shader - checks for compatibility with the model's mesh */
void wrm_render_updateModelShader(wrm_Handle model, wrm_Handle shader);
/* Associates models `child` and `parent` as such */
void wrm_render_addChildModel(wrm_Handle parent, wrm_Handle child);
/* Orphans model `child` from `parent` */
void wrm_render_removeChildModel(wrm_Handle parent, wrm_Handle child);

// camera-related

/* unusable at the moment; might be used later for projects where multiple cameras may be required */
// wrm_Option_Handle wrm_render_createCamera();
/* Updates the viewing camera */
void wrm_render_updateCamera(float pitch, float yaw, float fov, float offset, const vec3 position);
/* Gets the render's camera data, stores results in the provided pointers */
void wrm_render_getCameraData(float *pitch, float *yaw, float *fov, float *offset, vec3 position);


/* Self-explanatory RGBA functions: inlined because they are very small */

inline wrm_RGBA wrm_RGBA_fromRGBAi(wrm_RGBAi rgbai) 
{
    return (u32)rgbai.r << 24 | (u32)rgbai.g << 16 | (u32)rgbai.b << 8 | (u32)rgbai.a;
}

inline wrm_RGBA wrm_RGBA_fromRGBAf(wrm_RGBAf rgbaf)
{
    return (u32)(rgbaf.r * 255.0) << 24 | (u32)(rgbaf.g * 255.0) << 24 | (u32)(rgbaf.b * 255.0) << 24 | (u32)(rgbaf.a * 255.0);
}

inline wrm_RGBAi wrm_RGBAi_fromRGBA(wrm_RGBA rgba)
{
    return (wrm_RGBAi){
        .r = (u8)((rgba & WRM_RGBA_R_BITS) >> 24),
        .g = (u8)((rgba & WRM_RGBA_G_BITS) >> 16),
        .b = (u8)((rgba & WRM_RGBA_B_BITS) >> 8),
        .a = (u8)(rgba & WRM_RGBA_A_BITS)
    };
}

inline wrm_RGBAi wrm_RGBAi_fromRGBAf(wrm_RGBAf rgbaf)
{
    return wrm_RGBAi_fromRGBA(wrm_RGBA_fromRGBAf(rgbaf));
}

inline wrm_RGBAf wrm_RGBAf_fromRGBA(wrm_RGBA rgba)
{
    return (wrm_RGBAf) {
        .r = (float)((rgba & WRM_RGBA_R_BITS) >> 24) / 255.0f,
        .g = (float)((rgba & WRM_RGBA_G_BITS) >> 16) / 255.0f,
        .b = (float)((rgba & WRM_RGBA_B_BITS) >> 8) / 255.0f,
        .a = (float)(rgba & WRM_RGBA_A_BITS) / 255.0f
    };
}

inline wrm_RGBAf wrm_RGBAf_fromRGBAi(wrm_RGBAi rgbai)
{
    return wrm_RGBAf_fromRGBA(wrm_RGBA_fromRGBAi(rgbai));
}

# ifdef WRM_RENDER_IMPLEMENTATION

#include "wrm/common.h"
#include "wrm/memory.h"
#include "stb/stb_image.h"
#include "glad/glad.h"

/*
Internal type definitions
*/

// shader GL data, plus requirements of meshes rendered with it
typedef struct wrm_Shader {
    bool needs_col;
    bool needs_tex;
    GLuint vert;
    GLuint frag;
    GLuint program;
} wrm_Shader;

typedef struct wrm_Texture {
    GLuint gl_tex;
    // mipmap settings?
    // filter settings?
    u32 w;
    u32 h;
} wrm_Texture;

typedef struct wrm_Mesh {
    GLuint vao;
    GLuint pos_vbo;
    GLuint uv_vbo;
    GLuint col_vbo;
    GLuint ebo;
    size_t count;
    GLuint mode;
    bool cw;
} wrm_Mesh;

typedef struct wrm_Model {
    vec3 pos;
    vec3 rot;
    vec3 scale;

    wrm_Handle mesh;
    wrm_Handle texture; // only used when the model has a textured mesh; for now, meshes only use a single texture
    wrm_Handle shader;

    wrm_Handle parent;
    u8 child_count;
    wrm_Handle children[WRM_MODEL_CHILD_LIMIT];

    bool show_children;
    bool is_visible;
    bool is_ui; // whether the model should be rendered as part of the 3d pass or the 2d pass
} wrm_Model;

// camera data
typedef struct wrm_Camera {
    float pitch;
    float yaw;
    float offset; // distance forward (positive) or backward (negative) along `facing` from `pos`: used for 3rd-person controls
    float fov;
    vec3 pos;
} wrm_Camera;

// data needed to render a model
typedef struct wrm_Render_Data {
    mat4 transform;
    wrm_Handle mesh;
    wrm_Handle shader;
    wrm_Handle texture;
    wrm_Handle src_model;
} wrm_Model_Data;

DEFINE_OPTION(GLuint, GLuint);

DEFINE_LIST(wrm_Model_Data, Model_Data);

/*
Constants
*/

// windowing constants

const u32 WRM_DEFAULT_WINDOW_HEIGHT = 640;
const u32 WRM_DEFAULT_WINDOW_WIDTH = 800;

const u32 WRM_MESH_TRIANGLE = GL_TRIANGLES;
const u32 WRM_MESH_STRIP = GL_TRIANGLE_STRIP;
const u32 WRM_MESH_FAN = GL_TRIANGLE_FAN;

// resource constants
typedef enum wrm_render_Resource_Type {
    WRM_RENDER_RESOURCE_MODEL, 
    WRM_RENDER_RESOURCE_MESH, 
    WRM_RENDER_RESOURCE_SHADER, 
    WRM_RENDER_RESOURCE_TEXTURE
} wrm_render_Resource_Type;

// general rendering constants
internal const float WRM_NEAR_CLIP_DISTANCE = 0.001f;
internal const float WRM_FAR_CLIP_DISTANCE = 1000.0f;
// shader constants

wrm_Shader_Defaults wrm_shader_defaults;

internal const u32 WRM_SHADER_ATTRIB_POS_LOC = 0;
internal const u32 WRM_SHADER_ATTRIB_COL_LOC = 1;
internal const u32 WRM_SHADER_ATTRIB_UV_LOC = 2;
// internal const u32 WRM_SHADER_ATTRIB_NORM_LOC = 3; // unused (yet)

internal const char *WRM_SHADER_DEFAULT_COL_V_TEXT = {
"#version 330 core\n"
"layout (location = 0) in vec3 v_pos;\n" // positions are location 0
"layout (location = 1) in vec4 v_col;\n" // colors are location 1
"uniform mat4 model;\n"
"uniform mat4 persp;\n"
"uniform mat4 view;\n"
"out vec4 col;\n" // specify a color output to the fragment shader
"void main()\n"
"{\n"
"    gl_Position = persp * view * model * vec4(v_pos, 1.0);\n"
"    col = v_col;\n"
"}\n"
};
internal const char *WRM_SHADER_DEFAULT_COL_F_TEXT = {
"#version 330 core\n"
"in vec4 col;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = col;\n"
"}\n"
};

// texture

internal const char *WRM_SHADER_DEFAULT_TEX_V_TEXT = {
"#version 330 core\n"
"layout (location = 0) in vec3 v_pos;\n" // positions are location 0
"layout (location = 2) in vec2 v_uv;\n"  // uvs are location 2
"uniform mat4 model;\n"
"uniform mat4 persp;\n"
"uniform mat4 view;\n"
"out vec2 uv;\n" // specify a uv for the fragment shader
"void main()\n"
"{\n"
"    gl_Position = persp * view * model * vec4(v_pos, 1.0);\n" 
"    uv = v_uv;\n"
"}\n"
};
internal const char *WRM_SHADER_DEFAULT_TEX_F_TEXT = {
"#version 330 core\n"
"in vec2 uv;\n"
"uniform sampler2D tex;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = texture(tex, uv);\n"
"}\n"
};

// both
internal const char *WRM_SHADER_DEFAULT_BOTH_V_TEXT = {
"#version 330 core\n"
"layout (location = 0) in vec3 v_pos;\n" // positions are location 0
"layout (location = 1) in vec4 v_col;\n"
"layout (location = 2) in vec2 v_uv;\n"  // uvs are location 2
"uniform mat4 model;\n"
"uniform mat4 persp;\n"
"uniform mat4 view;\n"
"out vec4 col;\n" // specify a color for the fragment shader
"out vec2 uv;\n" // specify a uv for the fragment shader\n"
"void main()\n"
"{\n"
"    gl_Position = persp * view * model * vec4(v_pos, 1.0);\n"
"    col = v_col;\n" 
"    uv = v_uv;\n"
"}\n"
};
internal const char *WRM_SHADER_DEFAULT_BOTH_F_TEXT = {
"#version 330 core\n"
"in vec4 col;\n"
"in vec2 uv;\n"
"uniform sampler2D tex;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = texture(tex, uv) * col;\n"
"}\n"
};

// default meshes
wrm_Mesh_Data default_color_mesh_data = {
    .positions = (float[]) {
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
    },
    .colors = (float[]) {
        0.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
    },
    .uvs = NULL,
    .indices = (u32[]) {
        0, 1, 3, 0, 3, 2,
        1, 5, 7, 1, 7, 3,
        5, 4, 6, 5, 6, 7,
        4, 0, 2, 4, 2, 6,
        4, 5, 1, 4, 1, 0,
        2, 3, 7, 2, 7, 6,
    },
    .cw = true,
    .idx_cnt = 36,
    .vtx_cnt = 8,
    .mode = WRM_MESH_TRIANGLE,
};

wrm_Mesh_Data default_texture_mesh_data = {
    .positions = (float[]) {
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f, -0.5f,

         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
    },
    .colors = (float[]) {
        0.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        
        0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,

        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,

        1.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,

        0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
    },
    .uvs = (float[]) {
        0.0f, 0.0f, 1.0f, 0.0f, 
        0.0f, 1.0f, 1.0f, 1.0f,

        0.0f, 0.0f, 1.0f, 0.0f, 
        0.0f, 1.0f, 1.0f, 1.0f,

        0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 1.0f, 0.0f, 
        0.0f, 1.0f, 1.0f, 1.0f,

        0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 
    },
    .indices = (u32[]) {
        0, 1, 3, 0, 3, 2, 
        4, 5, 7, 4, 7, 6, 
        8, 9, 11, 8, 11, 10,
        12, 13, 15, 12, 15, 14,
        16, 17, 19, 16, 19, 18,
        20, 21, 23, 20, 23, 22,
    },
    .cw = true,
    .idx_cnt = 36,
    .vtx_cnt = 24,
    .mode = WRM_MESH_TRIANGLE,
};

// pool constants

internal const u32 WRM_RENDER_POOL_INITIAL_CAPACITY = 20;

// list constants

internal const u32 WRM_RENDER_LIST_INITIAL_CAPACITY = 10;
internal const u32 WRM_RENDER_LIST_SCALE_FACTOR = 2;


/*
Internal helper declarations
*/

// initializes the internal renderer resource pools
internal void wrm_render_initLists(void);
// compiles an individual shader program of the given GL type (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER)
internal wrm_Option_GLuint wrm_render_compileShader(const char *shader_text, GLenum type);
// comparison function for sorting models: 
internal int wrm_render_compareModelData(const void *model1, const void *model2);
// creates a default shader for meshes with per-vertex colors, per-vertex uv's, and both
internal void wrm_render_createDefaultShaders(void);
// creates a default pink-and-black error texture
internal void wrm_render_createErrorTexture(void);
// creates a default test triangle
internal void wrm_render_createTestModel(void);
// creates a list from the pool of models, sorted by GL state changes
internal inline void wrm_render_prepareModels(bool ui);
// adds a model and its children recursively to the TBD list
internal inline void wrm_render_addModelAndChildren(wrm_Handle m_handle, mat4 parent_transform);
// checks whether resource handle `h` to a resource of type `t` is in use 
// `caller` should be the name of the calling function, and `context` should explain the context of the check for helpful error messages
internal inline bool wrm_render_exists(wrm_Handle h, wrm_render_Resource_Type t, const char *caller, const char *context);
// gets the view matrix from the current camera orientation
internal inline void wrm_render_getViewMatrix(mat4 view);
// sets the GL state before a draw call
internal inline void wrm_render_setGLStateAndDraw(wrm_Model_Data *curr, wrm_Model_Data *prev, mat4 view, mat4 persp, u32 *count, u32 *mode, bool *indexed);
// pack position, rotation, and scale into a transform matrix
internal inline void wrm_render_packTransform(vec3 pos, vec3 rot, vec3 scale, mat4 transform);


/*
Module global variables
*/

// Overall module status

internal bool wrm_render_is_initialized = false;
internal wrm_Settings wrm_render_settings;

// SDL data 

internal SDL_Window *wrm_window = NULL; // SDL window 
internal SDL_GLContext wrm_gl_context; // gl context obtained from SDL

// GL data

internal wrm_RGBAf wrm_bg_color; // background color
internal int wrm_window_height;
internal int wrm_window_width;
internal vec3 wrm_world_up = {0.0f, 1.0f, 0.0f};
internal wrm_Camera wrm_camera;

// resource pools

internal wrm_Pool wrm_shaders;
internal wrm_Pool wrm_meshes;
internal wrm_Pool wrm_textures;
internal wrm_Pool wrm_models;

bool wrm_show_ui;
bool wrm_debug_frame;
internal u32 wrm_ui_count;

/* a list of models to be drawn (used solely in render_draw() )*/
internal wrm_List_Model_Data wrm_models_tbd;

/*
Module function definitions
*/


// module

bool wrm_render_init(const wrm_Settings *s, const wrm_Window_Data *data)
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
    wrm_render_initLists();
    if(wrm_render_settings.verbose) printf("Render: created resource pools\n");

    // add default resources to each list: the handle value 0 refers to these
    // setup default shaders
    wrm_render_createDefaultShaders();
    // setup default texture
    wrm_render_createErrorTexture();
    // setup default mesh (creates default model as well)
    wrm_render_createTestModel();
    if(wrm_render_settings.verbose) printf("Render: created default resources\n");

    // initialize GL data
    wrm_bg_color = data->background;
    glViewport(0, 0, wrm_window_width, wrm_window_height);

    // initialize camera
    wrm_camera = (wrm_Camera){
        .fov = 70.0f,
        .offset = 0.0f,
        .pitch = 0.0f,
        .yaw = 0.0f
    };

    wrm_render_is_initialized = true;
    wrm_debug_frame = false;
    return true;
}

void wrm_render_quit(void)
{
    if(!wrm_render_is_initialized) return;

    wrm_Pool_delete(&wrm_shaders);
    wrm_Pool_delete(&wrm_textures);
    wrm_Pool_delete(&wrm_meshes);
    wrm_Pool_delete(&wrm_models);

    free(wrm_models_tbd.data);

    SDL_GL_DeleteContext(wrm_gl_context);
    
    if(wrm_window) {
        SDL_DestroyWindow(wrm_window);
        wrm_window = NULL;
    }

    SDL_Quit();
}

void wrm_render_draw(float delta_time) 
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    // clear the screen
    glClearColor(wrm_bg_color.r, wrm_bg_color.g, wrm_bg_color.b, wrm_bg_color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // handle camera and get view matrix
    mat4 view;
    wrm_render_getViewMatrix(view);

    // get the perspective projection matrix (account for changes in window dimensions and camera fov)
    mat4 persp;
    float aspect_ratio = (float) wrm_window_width / (float) wrm_window_height;
    glm_perspective(wrm_camera.fov, aspect_ratio, WRM_NEAR_CLIP_DISTANCE, WRM_FAR_CLIP_DISTANCE, persp);

    // prepare a list of models for rendering
    wrm_render_prepareModels(false);
    
    // initialize GL state and tracking of changes
    wrm_Model_Data *prev = NULL;
    wrm_Model_Data *curr = wrm_models_tbd.data;
    u32 count = 0;
    u32 mode = 0;
    bool indexed = false;

    if(wrm_debug_frame) {
        printf("\nFRAME DRAW DATA:\n\n3D PASS (%u model%s to be drawn):\n", wrm_models_tbd.len, wrm_models_tbd.len > 1 ? "s" : "");
    }

    // render all the models to backbuffer
    for(int i = 0; i < wrm_models_tbd.len; i++) {
        if(wrm_debug_frame) wrm_render_printModelData(wrm_models_tbd.data[i].src_model);
        wrm_render_setGLStateAndDraw(curr, prev, view, persp, &count, &mode, &indexed);

        prev = curr;
        curr++;
    }

    // space for future post-processing effects

    // UI rendering pass
    wrm_render_prepareModels(true);

    // draw regardless of camera orientation
    glm_mat4_identity(view);
    // use orthographic projection
    glm_ortho(0.0f, wrm_window_width, 0.0f, wrm_window_height, 0.0f, 1.0f, persp);
    glDisable(GL_DEPTH_TEST);

    prev = NULL;
    curr = wrm_models_tbd.data;
    
    if(wrm_debug_frame) {
        printf("\n2D (UI) PASS (%u model%s to be drawn):\n", wrm_models_tbd.len, wrm_models_tbd.len > 1 ? "s" : "");
    }
    
    for(int i = 0; i < wrm_models_tbd.len; i++) {
        if(wrm_debug_frame) wrm_render_printModelData(wrm_models_tbd.data[i].src_model);
        wrm_render_setGLStateAndDraw(curr, prev, view, persp, &count, &mode, &indexed);

        prev = curr;
        curr++;
    }

    if(wrm_debug_frame) wrm_debug_frame = false;

    // swap the buffers to present the completed frame
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

void wrm_render_printDebugData(void)
{
    wrm_debug_frame = true;
    printf("DEBUG: Render:\n\nINTERNAL DATA\n\n");

    printf("Shaders: %zu total (memory for %zu): {\n", wrm_shaders.used, wrm_shaders.cap);
    for(u32 i = 0; i < wrm_shaders.cap; i++) {
        if(wrm_shaders.is_used[i]) {
            wrm_render_printShaderData(i);
        }
    }
    printf("}\n");
    printf("Textures: %zu total (memory for %zu): {\n", wrm_textures.used, wrm_textures.cap);
    for(u32 i = 0; i < wrm_textures.cap; i++) {
        if(wrm_textures.is_used[i]) {
            wrm_render_printTextureData(i);
        }
    }
    printf("}\n");
    printf("Meshes: %zu total (memory for %zu): {\n", wrm_meshes.used, wrm_meshes.cap);
    for(u32 i = 0; i < wrm_meshes.cap; i++) {
        if(wrm_meshes.is_used[i]) {
            wrm_render_printMeshData(i);
        }
    }
    printf("}\n");  
    printf("Models: %zu total (memory for %zu)\n", wrm_models.used, wrm_models.cap);
    for(u32 i = 0; i < wrm_models.cap; i++) {
        if(wrm_models.is_used[i]) {
            wrm_render_printModelData(i);
        }
    }
    printf("}\n");
}

void wrm_render_printShaderData(wrm_Handle shader)
{
    if(!wrm_render_exists(shader, WRM_RENDER_RESOURCE_SHADER, "printShaderData()", "")) return;
    
    wrm_Shader *s = wrm_Pool_dataAs(wrm_shaders, wrm_Shader);

    u32 i = shader;
    printf(
        "[%u]: { needs_col: %s, needs_tex: %s, vert: %u, frag: %u, program: %u }\n", 
        i,
        s[i].needs_col ? "true" : "false", 
        s[i].needs_tex ? "true" : "false",
        s[i].vert,
        s[i].frag,
        s[i].program
    );
}

void wrm_render_printTextureData(wrm_Handle texture)
{
    if(!wrm_render_exists(texture, WRM_RENDER_RESOURCE_TEXTURE, "printTextureData()", "")) return;
    wrm_Texture *t = wrm_Pool_dataAs(wrm_textures, wrm_Texture);
    u32 i = texture;
    printf(
        "[%u]: { gl_tex: %u, h: %u, w: %u }\n", 
        i,
        t[i].gl_tex,
        t[i].h,
        t[i].w
    );
}

void wrm_render_printMeshData(wrm_Handle mesh)
{
    if(!wrm_render_exists(mesh, WRM_RENDER_RESOURCE_MESH, "printMeshData()", "")) return;

    wrm_Mesh *m = wrm_Pool_dataAs(wrm_meshes, wrm_Mesh);
    u32 i = mesh;
    printf(
        "[%u]: { vao: %u, pos_vbo: %u, col_vbo: %u, uv_vbo: %u, ebo: %u, count: %zu, cw: %s, mode: %u }\n", 
        i,
        m[i].vao,
        m[i].pos_vbo,
        m[i].col_vbo, 
        m[i].uv_vbo,
        m[i].ebo,
        m[i].count,
        m[i].cw ? "true" : "false",
        m[i].mode
    );
}

void wrm_render_printModelData(wrm_Handle model)
{
    if(!wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, "printModelData()", "")) return;

    wrm_Model *m = wrm_Pool_dataAs(wrm_meshes, wrm_Model);
    u32 i = model;
    printf(
        "[%u]: { \n"
        "   shader: %u, texture: %u, mesh: %u, is_ui: %s, is_visible: %s,\n"
        "   pos: < %.2f %.2f %.2f >, rot: < %.2f %.2f %.2f >, scale: < %.2f %.2f %.2f >\n"
        "   parent: %u, child_count: %u, show_children: %s,\n"
        "   children: [ "
        , 
        i, 
        m[i].shader,
        m[i].texture,
        m[i].mesh,
        m[i].is_ui ? "true" : "false",
        m[i].is_visible ? "true" : "false",
        m[i].pos[0], m[i].pos[1], m[i].pos[2],
        m[i].rot[0], m[i].rot[1], m[i].rot[2],
        m[i].scale[0], m[i].scale[1], m[i].scale[2],
        m[i].parent,
        m[i].child_count,
        m[i].show_children ? "true" : "false"
    );
    for(u8 j = 0; j < WRM_MODEL_CHILD_LIMIT; j++) {
        printf("%u, ", m[i].children[j]);
    }
    printf("]   }\n");
}


void wrm_render_setUIShown(bool show_ui)
{
    
}

// shader

wrm_Option_Handle wrm_render_createShader(const char *vert_text, const char *frag_text, bool needs_col, bool needs_tex)
{
    wrm_Option_Handle pool_result = wrm_Pool_getSlot(&wrm_shaders);

    if(!pool_result.exists) return pool_result;

    wrm_Shader s;
    s.needs_col = needs_col;
    s.needs_tex = needs_tex;

    // first compile the vertex and fragment shaders individually
    wrm_Option_GLuint result = wrm_render_compileShader(vert_text, GL_VERTEX_SHADER);
    if(!result.exists) {
        wrm_Pool_freeSlot(&wrm_shaders, pool_result.val);
        return (wrm_Option_Handle){.exists = false };
    }
    s.vert = result.val;
    
    result = wrm_render_compileShader(frag_text, GL_FRAGMENT_SHADER);
    if(!result.exists) {
        wrm_Pool_freeSlot(&wrm_shaders, pool_result.val);
        glDeleteShader(s.vert);
        return (wrm_Option_Handle){.exists = false };
    }
    s.frag = result.val;
    
    // then link them to form a program

    GLuint program = glCreateProgram();

    glAttachShader(program, s.vert);
    glAttachShader(program, s.frag);

    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if(success == GL_FALSE) {
        if(wrm_render_settings.errors) {
            GLint log_len = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);

            char *log_msg = malloc(log_len * sizeof(char));
            glGetProgramInfoLog(program, log_len, NULL, log_msg);
            fprintf(stderr, "ERROR: Render: failed to link shaders, GL error: %s\n", log_msg);
            free(log_msg);
        }

        glDeleteProgram(program);
        glDeleteShader(s.vert);
        glDeleteShader(s.frag);

        wrm_Pool_freeSlot(&wrm_shaders, pool_result.val);
        return (wrm_Option_Handle){ .exists = false };
    }

    glDetachShader(program, s.vert);
    glDetachShader(program, s.frag);

    s.program = program;

    if (s.needs_tex) {
    glUseProgram(program);
    GLint tex_uniform = glGetUniformLocation(program, "tex");
    if (tex_uniform != -1) {
        glUniform1i(tex_uniform, 0); // Assumes all your textured shaders use GL_TEXTURE0: can later extend to use multiple textures
    }
    glUseProgram(0); // Optional: Unbind the program
}

    ((wrm_Shader*)wrm_shaders.data)[pool_result.val] = s;
    return pool_result;
}

// texture

wrm_Option_Handle wrm_render_createTexture(const wrm_Texture_Data *data)
{
    wrm_Option_Handle result = wrm_Pool_getSlot(&wrm_textures);

    if(!result.exists) return result;

    bool font = data->font;

    GLuint texture;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLuint gl_format = font ? GL_RED : GL_RGBA;
    GLuint in_format = font ? GL_ALPHA : GL_RGBA;
    if(font) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(
        GL_TEXTURE_2D,  // texture target type
        0,              // detail level (for manually adding mipmaps; don't do this, generate them with glGenerateMipmap)
        gl_format,      // format OpenGL should store the image with
        data->width,    // width in pixels
        data->height,   // height in pixels
        0,              // border (weird legacy argument - borders should be set explicitly with glTexParameterxx)
        in_format,      // format of the incoming image data
        GL_UNSIGNED_BYTE,
        data->pixels
    );
    
    if(!font) { glGenerateMipmap(GL_TEXTURE_2D); }

    ((wrm_Texture*)wrm_textures.data)[result.val] = (wrm_Texture){
        .gl_tex = texture,
        .w = data->width,
        .h = data->height,
    };

    return result;
}

bool wrm_render_updateTexture(wrm_Handle texture, wrm_Texture_Data *data, u32 x, u32 y)
{
    if(!wrm_render_exists(texture, WRM_RENDER_RESOURCE_TEXTURE, "updateTexture()", "")) {
        return false;
    }

    wrm_Texture t = ((wrm_Texture*)wrm_textures.data)[texture];

    GLuint gl_format = data->font ? GL_ALPHA : GL_RGBA;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t.gl_tex);
    glTexSubImage2D(GL_TEXTURE, 0, x, y, data->width, data->height, gl_format, GL_UNSIGNED_BYTE, data->pixels);
    return true;
}

// mesh

wrm_Option_Handle wrm_render_createMesh(const wrm_Mesh_Data *data)
{
    if(!data) { return OPTION_NONE(Handle); }
    wrm_Option_Handle result = wrm_Pool_getSlot(&wrm_meshes);
    GLuint vtx_attrib;
    GLuint gl_draw = data->dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    u8 floats_per_pos = 3;
    u8 floats_per_col = 4;
    u8 floats_per_uv = 2;

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint pos_vbo = 0;
    glGenBuffers(1, &pos_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
    glBufferData(GL_ARRAY_BUFFER, data->vtx_cnt * floats_per_pos * sizeof(float), data->positions, gl_draw);

    vtx_attrib = WRM_SHADER_ATTRIB_POS_LOC;
    glVertexAttribPointer(vtx_attrib, floats_per_pos, GL_FLOAT, GL_FALSE, floats_per_pos * sizeof(float), (void*)0);
    glEnableVertexAttribArray(vtx_attrib);

    GLuint col_vbo = 0;
    if(data->colors) {
        glGenBuffers(1, &col_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, col_vbo);
        glBufferData(GL_ARRAY_BUFFER, data->vtx_cnt * floats_per_col * sizeof(float), data->colors, gl_draw);

        vtx_attrib = WRM_SHADER_ATTRIB_COL_LOC;
        glVertexAttribPointer(vtx_attrib, floats_per_col, GL_FLOAT, GL_FALSE, floats_per_col * sizeof(float), (void*)0);
        glEnableVertexAttribArray(vtx_attrib);
    }

    GLuint uv_vbo = 0;
    if(data->uvs) {
        glGenBuffers(1, &uv_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
        glBufferData(GL_ARRAY_BUFFER, data->vtx_cnt * 2 * sizeof(float), data->uvs, gl_draw);

        vtx_attrib = WRM_SHADER_ATTRIB_UV_LOC;
        glVertexAttribPointer(vtx_attrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(vtx_attrib);
    }

    
    GLuint ebo = 0;
    if(data->indices) {
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        // handle indices for different drawing modes

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data->idx_cnt * sizeof(u32), data->indices, gl_draw);
    }
    

    wrm_Mesh m = {
        .vao = vao,
        .pos_vbo = pos_vbo,
        .col_vbo = col_vbo,
        .uv_vbo = uv_vbo,
        .ebo = ebo,
        .count = data->idx_cnt,
        .mode = data->mode, 
        .cw = data->cw,
    };

    ((wrm_Mesh*)wrm_meshes.data)[result.val] = m;
    return result;
}

wrm_Option_Handle wrm_render_cloneMesh(wrm_Handle mesh)
{
    // TODO: look up how to duplicate data that has been sent to the GPU
    return OPTION_NONE(Handle);
}

bool wrm_render_updateMesh(wrm_Handle mesh, const wrm_Mesh_Data *data)
{
    return true;
}

// model

wrm_Option_Handle wrm_render_createModel(const wrm_Model_Data *data, wrm_Handle parent, bool use_default_shader)
{
    wrm_Option_Handle result = wrm_Pool_getSlot(&wrm_models);

     
    if(!result.exists) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: failed to create model\n"); }
        return result;
    }

    if(parent && !wrm_render_exists(parent, WRM_RENDER_RESOURCE_MODEL, "createModel()", "(parent)")) {
        wrm_Pool_freeSlot(&wrm_models, result.val);
        return OPTION_NONE(Handle);
    }

    // do some fun pointer arithmetic to get the allocated model

    wrm_Model* model = wrm_Pool_dataAs(wrm_models, wrm_Model) + result.val;


    // explicitly set up parent / children
    if(parent) wrm_render_addChildModel(parent, result.val);
    model->child_count = 0;
    for(u8 i = 0; i < WRM_MODEL_CHILD_LIMIT; i++) {
        model->children[i] = 0;
    }

    
    if(use_default_shader) {
        
        if(mesh.col_vbo && mesh.uv_vbo) {
            model->shader = wrm_shader_defaults.both;
        }
        else if(mesh.col_vbo) {
            model->shader = wrm_shader_defaults.color;
        }
        else if(mesh.uv_vbo) {
            model->shader = wrm_shader_defaults.texture;
        }
        else {
            if(wrm_render_settings.errors) wrm_error("Render: createModel()", "No suitable default shader for mesh [%u] found\n", data->mesh);
            wrm_Pool_freeSlot(&wrm_models, result.val);
            return OPTION_NONE(Handle);
        }
    }

    if(!wrm_render_exists(model->shader, WRM_RENDER_RESOURCE_SHADER, "createModel()", "(shader)")) {
        wrm_Pool_freeSlot(&wrm_models, result.val);
        return OPTION_NONE(Handle);
    }
    
    

    if(model->is_ui) { wrm_ui_count++; }

    if(wrm_render_settings.verbose) wrm_render_printModelData(result.val);
    return result;
}

bool wrm_render_getModel(wrm_Handle model, wrm_Model *dest)
{
    if(!wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, "getModel()", "")) {
        return false;
    }
    *dest = ((wrm_Model*)wrm_models.data)[model];
    return true;
}

void wrm_render_updateModelTransform(wrm_Handle model, const vec3 pos, const vec3 rot, const vec3 scale)
{
    if(!wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, "updateModelTransform()", "")) {
        return;
    }
    wrm_Model *data = (wrm_Model*)wrm_models.data;

    data[model].pos[0] = pos[0];
    data[model].pos[1] = pos[1];
    data[model].pos[2] = pos[2];

    data[model].rot[0] = rot[0];
    data[model].rot[1] = rot[1];
    data[model].rot[2] = rot[2];

    data[model].scale[0] = scale[0];
    data[model].scale[1] = scale[1];
    data[model].scale[2] = scale[2];
}

void wrm_render_updateModelMesh(wrm_Handle model, wrm_Handle mesh) 
{
    const char *caller = "updateModelMesh()";
    if(wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, caller, "(model)") && wrm_render_exists(mesh, WRM_RENDER_RESOURCE_MESH, caller, "(mesh)")) {
        ((wrm_Model*)wrm_models.data)[model].mesh = mesh;
    }
}

void wrm_render_updateModelTexture(wrm_Handle model, wrm_Handle texture)
{
    const char *caller = "updateModelTexture()";
    if(wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, caller, "(model)") && wrm_render_exists(texture, WRM_RENDER_RESOURCE_TEXTURE, caller, "(texture)")) {
        ((wrm_Model*)wrm_models.data)[model].texture = texture;
    }
}

void wrm_render_updateModelShader(wrm_Handle model, wrm_Handle shader)
{
    const char *caller = "updateModelShader()";
    if(!wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, caller, "(model)") && wrm_render_exists(shader, WRM_RENDER_RESOURCE_SHADER, caller, "(shader)")) {
        return;
    }

    wrm_Model* mod = wrm_Pool_dataAs(wrm_models, wrm_Model) + model;
    wrm_Shader sh = wrm_Pool_dataAs(wrm_shaders, wrm_Shader)[shader];
    wrm_Mesh mesh = wrm_Pool_dataAs(wrm_meshes, wrm_Mesh)[mod->mesh];

    // ensure the shader and mesh are compatible
    if( (sh.needs_col && !mesh.col_vbo) || (sh.needs_tex && !mesh.uv_vbo)) {
        if(wrm_render_settings.errors) wrm_error("Render: createModel()", "Mesh [%u] does not meet shader [%u] data requirements\n", mod->mesh, shader);
        return OPTION_NONE(Handle);
    }

    mod->shader = shader;
}

void wrm_render_updateModel(wrm_Handle model, const wrm_Model *data)
{
    if(!wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, "updateModel()", "")) {
        return;
    }

    wrm_render_updateModelTransform(model, data->pos, data->rot, data->scale);
    wrm_render_updateModelMesh(model, data->mesh);
    wrm_render_updateModelTexture(model, data->texture);
    wrm_render_updateModelShader(model, data->shader);
}

void wrm_render_addChildModel(wrm_Handle parent, wrm_Handle child)
{
    char * caller = "addChildModel()";
    wrm_render_Resource_Type type = WRM_RENDER_RESOURCE_MODEL;
    if(!(wrm_render_exists(parent, type, caller, "(parent)") && wrm_render_exists(child, type, caller, "(child)"))) {
        return;
    }

    wrm_Model *m = wrm_Pool_dataAs(wrm_models, wrm_Model);

    if(m[parent].child_count == WRM_MODEL_CHILD_LIMIT) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: cannot add another child to model [%u] (limit reached!)\n", parent); }
        return;
    }

    if(m[child].parent) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: cannot add parent to model [%u] (already has parent!)\n", child); }
        return;
    }

    for(u8 i = 0; i < m[parent].child_count; i++) {
        if(m[parent].children[i] == child) {
            if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: cannot add child [%u] to model [%u] (already added!)\n", child, parent); }
            return;
        }
    }

    if(parent) m[parent].children[m[parent].child_count++] = child;
    m[child].parent = parent;
}

void wrm_render_removeChildModel(wrm_Handle parent, wrm_Handle child)
{
    char * caller = "removeChildModel()";
    wrm_render_Resource_Type type = WRM_RENDER_RESOURCE_MODEL;
    if(!(wrm_render_exists(parent, type, caller, "(parent)") && wrm_render_exists(child, type, caller, "(child)"))) {
        return;
    }

    wrm_Model *m = wrm_Pool_dataAs(wrm_models, wrm_Model);

    if(m[parent].child_count == 0) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: cannot remove child from model [%u] (has no children!)\n", parent); }
        return;
    }

    if(!m[child].parent) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: cannot remove parent from model [%u] (has no parent!)\n", child); }
        return;
    }

    if(m[child].parent != parent) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: cannot remove parent [%u] from model [%u] (has a different parent!)\n", parent, child); }
        return;
    }   

    
    
    // find the child
    u8 idx;
    for(idx = 0; idx < m[parent].child_count; idx++) {
        if(m[parent].children[idx] == child) break;
    }
    if(idx == WRM_MODEL_CHILD_LIMIT) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: cannot remove child [%u] from model [%u] (already removed!)\n", child, parent); }
        return;
    }

    m[child].parent = 0;

    m[parent].children[idx] = 0;
    m[parent].child_count--;
    
    while(idx < m[parent].child_count) {
        m[parent].children[idx] = m[parent].children[idx + 1];
        idx++;
    }
    m[parent].children[idx] = 0;
}

// camera

void wrm_render_updateCamera(float pitch, float yaw, float fov, float offset, const vec3 pos)
{
    wrm_camera.pitch = pitch;
    wrm_camera.yaw = yaw;
    wrm_camera.fov = fov;
    wrm_camera.offset = offset;
    
    wrm_camera.pos[0] = pos[0];
    wrm_camera.pos[1] = pos[1];
    wrm_camera.pos[2] = pos[2];
}

void wrm_render_getCameraData(float *pitch, float *yaw, float *fov, float *offset, vec3 position)
{
    if(pitch) *pitch = wrm_camera.pitch;
    if(yaw) *yaw = wrm_camera.yaw;
    if(fov) *fov = wrm_camera.fov;
    if(offset) *offset = wrm_camera.offset;

    if(position) {
        position[0] = wrm_camera.pos[0];
        position[1] = wrm_camera.pos[1];
        position[2] = wrm_camera.pos[2];
    }
}


// these are all defined in the header, but this ensures a compiler symbol is actually emitted for them

wrm_RGBAf wrm_RGBAf_fromRGBAi(wrm_RGBAi rgbai);

wrm_RGBAf wrm_RGBAf_fromRGBA(wrm_RGBA rgba);

wrm_RGBAi wrm_RGBAi_fromRGBAf(wrm_RGBAf rgbaf);

wrm_RGBAi wrm_RGBAi_fromRGBA(wrm_RGBA rgba);

wrm_RGBA wrm_RGBA_fromRGBAi(wrm_RGBAi rgbai);

wrm_RGBA wrm_RGBA_fromRGBAf(wrm_RGBAf rgbaf);

/*
Internal helper definitions
*/

internal void wrm_render_initLists(void)
{
    wrm_Pool_init(&wrm_shaders, WRM_RENDER_POOL_INITIAL_CAPACITY, sizeof(wrm_Shader));
    wrm_Pool_init(&wrm_textures, WRM_RENDER_POOL_INITIAL_CAPACITY, sizeof(wrm_Texture));
    wrm_Pool_init(&wrm_meshes, WRM_RENDER_POOL_INITIAL_CAPACITY, sizeof(wrm_Mesh));
    wrm_Pool_init(&wrm_models, WRM_RENDER_POOL_INITIAL_CAPACITY, sizeof(wrm_Model));

    wrm_models_tbd = (wrm_List_Model_Data) {
        .cap = WRM_RENDER_LIST_INITIAL_CAPACITY, 
        .len = 0, 
        .data = (wrm_Model_Data*)calloc(WRM_RENDER_LIST_INITIAL_CAPACITY, sizeof(wrm_Model_Data))
    };

    wrm_ui_count = 0;
}


internal wrm_Option_GLuint wrm_render_compileShader(const char *shader_text, GLenum type) 
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_text, NULL);
    glCompileShader(shader);

    // error checking
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(success == GL_FALSE) {
        if(wrm_render_settings.errors) {
            GLint log_len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

            char *log_msg = malloc(log_len * sizeof(char));
            glGetShaderInfoLog(shader, log_len, NULL, log_msg);

            fprintf(stderr, "ERROR: Render: failed to compile shader, GL error: %s\n Shader source: %s\n", log_msg, shader_text);
            free(log_msg);
        }

        glDeleteShader(shader);

        return (wrm_Option_GLuint){ .exists = false };
    }

    return (wrm_Option_GLuint){ .exists = true, .GLuint_val = shader };
}

internal int wrm_render_compareModelData(const void *model1, const void *model2)
{
    const wrm_Model_Data *m1 = (wrm_Model_Data*)model1;
    const wrm_Model_Data *m2 = (wrm_Model_Data*)model2;

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

internal void wrm_render_createDefaultShaders(void)
{
    wrm_Option_Handle result; 
    result = wrm_render_createShader(WRM_SHADER_DEFAULT_COL_V_TEXT, WRM_SHADER_DEFAULT_COL_F_TEXT, true, false);
    if(!result.exists) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: failed to create default color shader\n"); }
    }
    wrm_shader_defaults.color = result.val;

    result = wrm_render_createShader(WRM_SHADER_DEFAULT_TEX_V_TEXT, WRM_SHADER_DEFAULT_TEX_F_TEXT, false, true);
    if(!result.exists) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: failed to create default texture shader\n"); }
    }
    wrm_shader_defaults.texture = result.val;

    result = wrm_render_createShader(WRM_SHADER_DEFAULT_BOTH_V_TEXT, WRM_SHADER_DEFAULT_BOTH_F_TEXT, true, true);
    if(!result.exists) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: failed to create default color + texture shader\n"); }
    }
    wrm_shader_defaults.both  = result.val;
}

internal void wrm_render_createErrorTexture(void)
{
    wrm_RGBAi p = wrm_RGBAi_fromRGBA(WRM_RGBA_PURPLE);
    wrm_RGBAi b = wrm_RGBAi_fromRGBA(WRM_RGBA_BLACK);

    u8 pixels[] = {
        p.r, p.g, p.b, p.a,
        b.r, b.g, b.b, b.a,
        b.r, b.g, b.b, b.a,
        p.r, p.g, p.b, p.a
    };

    wrm_Texture_Data t = (wrm_Texture_Data) {
        .pixels = pixels,
        .height = 2,
        .width = 2
    };

    wrm_Option_Handle texture = wrm_render_createTexture(&t);
    if(!texture.exists) {
        if(wrm_render_settings.errors) fprintf(stdout, "ERROR: Render: failed to create error texture\n");
        return;
    }

    if(wrm_render_settings.verbose) printf("Render: created error texture\n");
}

internal void wrm_render_createTestModel(void)
{
    wrm_Option_Handle mesh = wrm_render_createMesh(&default_texture_mesh_data);
    if(!mesh.exists) {
        if(wrm_render_settings.errors) fprintf(stdout, "ERROR: Render: failed to create test triangle mesh\n");
        return;
    }
    if(wrm_render_settings.verbose) printf("Render: Created test triangle mesh (handle=%d)\n", mesh.val);

    wrm_Model model_data = {
        .pos = {1.0f, 0.0f, 0.0f},
        .rot = {0.0f, 0.0f, 0.0f},
        .scale = {1.0f, 1.0f, 1.0f},
        .mesh = mesh.val,
        .texture = 0,
        .shader = wrm_shader_defaults.texture,
        .is_visible = wrm_render_settings.test,
        .is_ui = false
    };
    wrm_Option_Handle model = wrm_render_createModel(&model_data, 0, false);
    if(!model.exists) {
        if(wrm_render_settings.errors) wrm_error("Render", "failed to create test triangle model");
        return;
    }
    if(wrm_render_settings.verbose) printf("Render: Created test triangle model (handle=%d)\n", model.val);
}

internal inline bool wrm_render_exists(wrm_Handle h, wrm_render_Resource_Type t, const char *caller, const char *context)
{
    bool result;
    const char *type;
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
            if(wrm_render_settings.errors) fprintf(stderr, "ERROR: Render: internal: exists(): invalid resource type [%d]\n", t);
            return false;
    }

    if(!result && wrm_render_settings.errors) fprintf(stderr, "ERROR: Render: %s: %s [%u] does not exist %s\n", caller, type, h, context);
    return result;
}

internal inline void wrm_render_prepareModels(bool ui_pass)
{
    u32 required_capacity = ui_pass ? wrm_ui_count : wrm_models.used - wrm_ui_count;

    // clear the list
    wrm_models_tbd.len = 0;

    wrm_Model *models = wrm_Pool_dataAs(wrm_models, wrm_Model);

    // ensure lists have enough capacity to hold all models
    if(wrm_models_tbd.cap < required_capacity) {
        if(!realloc(wrm_models_tbd.data, WRM_RENDER_LIST_SCALE_FACTOR * wrm_models_tbd.cap * sizeof(wrm_Model_Data))) {
            fprintf(stderr, "ERROR: Render: failed to allocate more memory for models to-be-drawn list\n");
            return;
        }
    }

    for(u32 i = 0; i < wrm_models.cap; i++) {
        if(wrm_models.is_used[i] && !models[i].parent && models[i].is_ui == ui_pass) {
            wrm_render_addModelAndChildren(i, NULL);
        }
    }

    if(wrm_models_tbd.len > 1 && !ui_pass) {
        qsort(wrm_models_tbd.data, wrm_models_tbd.len, sizeof(wrm_Model_Data), wrm_render_compareModelData);
    }
}

internal inline void wrm_render_addModelAndChildren(wrm_Handle m_handle, mat4 parent_transform)
{
    wrm_Model m = ((wrm_Model*)wrm_models.data)[m_handle];

    wrm_Model_Data data; 
    wrm_render_packTransform(m.pos, m.rot, m.scale, data.transform);
    if(parent_transform) {
        glm_mat4_mul(parent_transform, data.transform, data.transform);
    }

    if(m.is_visible) { 
        data.mesh = m.mesh;
        data.shader = m.shader;
        data.texture = m.texture;
        data.src_model = m_handle;
        wrm_models_tbd.data[wrm_models_tbd.len++] = data; 
    }
    
    if(!(m.child_count && m.show_children)) { return; }

    // add children in order
    for(u8 i = 0; i < m.child_count; i++) {
        wrm_render_addModelAndChildren(m.children[i], data.transform);
    }
}

internal inline void wrm_render_getViewMatrix(mat4 view)
{
    // update camera facing direction
    vec3 facing = {
        cosf(glm_rad(wrm_camera.yaw)) * cosf(glm_rad(wrm_camera.pitch)),
        sinf(glm_rad(wrm_camera.pitch)),
        sinf(glm_rad(wrm_camera.yaw)) * cosf(glm_rad(wrm_camera.pitch))
    };
    glm_normalize(facing);

    vec3 pos = {
        wrm_camera.pos[0],
        wrm_camera.pos[1],
        wrm_camera.pos[2],
    };
    
    // include offset in camera eye position
    vec3 eye;
    vec3 offset_vec;
    // If offset > 0 (backward): Subtract direction vector * offset from player position
    glm_vec3_scale(facing, wrm_camera.offset, offset_vec);
    glm_vec3_sub(pos, offset_vec, eye); 

    // get target point (can't just pass facing to lookAt())
    vec3 target;
    glm_vec3_add(eye, facing, target); 

    // get the view projection matrix
    glm_lookat(eye, target, wrm_world_up, view);
}

internal inline void wrm_render_setGLStateAndDraw(wrm_Model_Data *curr, wrm_Model_Data *prev, mat4 view, mat4 persp, u32 *count, u32 *mode, bool *indexed)
{
    if(!curr) return;
    wrm_Shader *s = (wrm_Shader*)wrm_shaders.data + curr->shader;
    
    
    if(!prev || curr->shader != prev->shader) {
        glUseProgram(s->program);
        GLint view_loc = glGetUniformLocation(s->program, "view");
        if(view_loc != -1) {
            glUniformMatrix4fv(view_loc, 1, GL_FALSE, (float*)view);
        }

        GLint persp_loc = glGetUniformLocation(s->program, "persp");
        if(persp_loc != -1) {
            glUniformMatrix4fv(persp_loc, 1, GL_FALSE, (float*)persp);
        }
    }
    
    if(!prev || curr->texture != prev->texture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ((wrm_Texture*)wrm_textures.data)[curr->texture].gl_tex);
    }

    if(!prev || curr->mesh != prev->mesh) {
        wrm_Mesh *m = (wrm_Mesh*)wrm_meshes.data + curr->mesh;
        glBindVertexArray(m->vao);
        glFrontFace(m->cw ? GL_CW : GL_CCW);
        *count = m->count;
        *mode = m->mode;
        *indexed = m->ebo;
    }
    
    GLint model_loc = glGetUniformLocation(s->program, "model");
    if(model_loc != -1) {
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, (float*)curr->transform);
    }

    if(*indexed) {
        glDrawElements(*mode, *count, GL_UNSIGNED_INT, NULL);
    }
    else {
        glDrawArrays(*mode, 0, *count);
    }
}

internal inline void wrm_render_packTransform(vec3 pos, vec3 rot, vec3 scale, mat4 transform)
{
    glm_mat4_identity(transform);
    glm_scale(transform, scale);

    glm_rotate(transform, glm_rad(rot[0]), GLM_XUP);
    glm_rotate(transform, glm_rad(rot[1]), GLM_YUP);
    glm_rotate(transform, glm_rad(rot[2]), GLM_ZUP);

    glm_translate(transform, pos);
}

# endif // end header implementation

#endif // end header
