#ifndef WRM_RENDER_H
#define WRM_RENDER_H

/*
File wrm-render.h

Created Oct 29, 2025 
by William R Mungas (wrm)

Version: 0.1.0 
(Last modified Nov 13, 2025)

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
Externally visible constants
*/

#define WRM_X 0
#define WRM_Y 1
#define WRM_Z 2

#define WRM_PITCH WRM_Z
#define WRM_YAW WRM_Y
#define WRM_ROLL WRM_X

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

/*
Type Declarations
*/

// window creation arguments
typedef struct wrm_Window_Data wrm_Window_Data; 

// simple shaders for various mesh types
typedef struct wrm_Shader_Defaults wrm_Shader_Defaults;

// single 32-bit integer rgba value
typedef u32 wrm_RGBA;
// collection of 4 bytes: r, g, b, a
typedef struct wrm_RGBAi wrm_RGBAi;
// collection of 4 floats: r, g, b, a: used for when OpenGL wants color channel values as floats in the range [0.0f, 1.0f]
typedef struct wrm_RGBAf wrm_RGBAf;
// data for creating a texture
typedef struct wrm_Texture_Data wrm_Texture_Data;

// Arguments for mesh creation
typedef struct wrm_Mesh_Data wrm_Mesh_Data;
// Enum of Mesh types
typedef enum wrm_Mesh_Type wrm_Mesh_Type;

// Arguments for model creation
typedef struct wrm_Model_Data wrm_Model_Data;

/*
Type definitions
*/


struct wrm_Window_Data {
    const char *name; // the name of the window
    i32 height_px; // the height of the window in pixels
    i32 width_px; // the width of the window in pixels
    bool is_resizable; // whether or not the window should be resizable
    wrm_RGBA background; // RGBA background color (packed as a 32-bit unsigned integer)
};

struct wrm_Shader_Defaults {
    wrm_Handle color; // default shader for a mesh with RGBA color attributes
    wrm_Handle texture; // default shader for a mesh with uv texture coordinates
    wrm_Handle both; // default shader for a mix of color and texture
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

struct wrm_Texture_Data {
    u8 *pixels; // either an array of [4 * width * height] bytes, or NULL (user will update later)
    u32 width; // width of the texture, in pixels
    u32 height; // height of the texture, in pixels
    u32 channels; // number of channels of pixel data (should be RGBA or RGB)
};

struct wrm_Mesh_Data {
    float *positions;       // position for each vertex
    float *colors;          // RGBA color for each vertex
    float *uvs;             // uv for each vertex
    u32 *indices;           // vertex indices
    size_t vtx_cnt;         // number of vertices for which we have data (independent of number of triangles)
    size_t idx_cnt;         // the number of indices in the mesh (3 * total tris for GL_TRIANGLES)
    u32 mode;
    bool cw;                // does the mesh use a clockwise winding order?
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

// default shaders - initialized within `render_init()`
extern wrm_Shader_Defaults wrm_shader_defaults;

/*
Module functions
*/

/* Initialize the renderer - opens a window with a GL context, sets up default shaders */
bool wrm_render_init(const wrm_Settings *settings, const wrm_Window_Data *data);
/* Shut down the renderer and clean up resources */
void wrm_render_quit(void);
/* Cleans up any unused/freed resources and renders all currently visible objects */
void wrm_render_draw(void);
/* Get the window created by the render - may be needed by other modules */
SDL_Window *wrm_render_getWindow(void);
/* Updates renderer on a window resize event */
void wrm_render_onWindowResize(void);
/* Prints debug info about current render state to standard output */
void wrm_render_printDebugData(void);
/* Set a flag for whether or not to show the GUI elements */
void wrm_render_setUIShown(bool show_ui);

// shader-related

/* Creates a shader program using the given shader source strings */
wrm_Option_Handle wrm_render_createShader(const char *vert, const char *frag, bool needs_col, bool needs_tex);
/* For debugging; prints a shader's data to `stdout` */
void wrm_render_printShaderData(wrm_Handle shader);
// texture-related

/* Creates a texture */
wrm_Option_Handle wrm_render_createTexture(const wrm_Texture_Data *data);
/* Update part of a texture from the given x and y offset with the given data */
bool wrm_render_updateTexture(wrm_Handle texture, wrm_Texture_Data *data, u32 x, u32 y);
/* For debugging; prints a stexture's data to `stdout` */
void wrm_render_printTextureData(wrm_Handle texture);

// mesh-related

/* Create a mesh */
wrm_Option_Handle wrm_render_createMesh(const wrm_Mesh_Data *data);
/* Clones an existing mesh (useful for changing data without affecting all instances of this mesh)*/
wrm_Option_Handle wrm_render_cloneMesh(wrm_Handle mesh);
/* Updates a mesh's data: IMPORTANT: will update ALL existing instances of this mesh */
bool wrm_render_updateMesh(wrm_Handle mesh, const wrm_Mesh_Data *data);
/* For debugging; prints a mesh's data to `stdout` */
void wrm_render_printMeshData(wrm_Handle mesh);

// model-related

/* Create a model - if use_default_shader is true, the renderer will attempt to select a default shader based on the mesh attributes */
wrm_Option_Handle wrm_render_createModel(const wrm_Model_Data *data, wrm_Handle parent, bool use_default_shader);
/* Update a model's transform data */
bool wrm_render_updateModelTransform(wrm_Handle model, const vec3 pos, const vec3 rot, const vec3 scale);
/* Update a model's mesh */
bool wrm_render_updateModelMesh(wrm_Handle model, wrm_Handle mesh);
/* Update a model's texture*/
bool wrm_render_updateModelTexture(wrm_Handle model, wrm_Handle texture);
/* Update a model's shader - checks for compatibility with the model's mesh */
bool wrm_render_updateModelShader(wrm_Handle model, wrm_Handle shader);
/* Associates models `child` and `parent` as such */
bool wrm_render_addChild(wrm_Handle parent, wrm_Handle child);
/* Orphans model `child` from `parent` */
bool wrm_render_removeChild(wrm_Handle parent, wrm_Handle child);
/* creates a default colored test triangle - for testing */
wrm_Option_Handle wrm_render_createTestTriangle(void);
/* creates a default error-textured test cube - for testing */
wrm_Option_Handle wrm_render_createTestCube(void);
/* For debugging; prints a model's data to `stdout` */
void wrm_render_printModelData(wrm_Handle model);

// camera-related

/* unusable at the moment; might be used later for projects where multiple cameras may be required */
// wrm_Option_Handle wrm_render_createCamera();
/* Updates the viewing camera; ignores any NULL values */
void wrm_render_updateCamera(float *fov, float *offset, const vec3 pos, const vec3 rot);
/* Gets the render's camera data, stores results in the provided pointers */
void wrm_render_getCameraData(float *fov, float *offset, vec3 pos, vec3 rot);
/* print debug information about the camera */
void wrm_render_printCameraData(void);



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

// vector functions

/* useful since cglm doesn't automatically provide const-correct copy for some reason */
inline void wrm_vec3_copy(const vec3 src, vec3 dest)
{
    if(src && dest && src != dest) {
        dest[WRM_X] = src[WRM_X];
        dest[WRM_Y] = src[WRM_Y];
        dest[WRM_Z] = src[WRM_Z];
    }
}
/* get forward, up, and right vectors from a given orientation vector: applies yaw->pitch->roll*/
void wrm_render_getOrientation(const vec3 rot, vec3 forward, vec3 up, vec3 right);
/* gets forward and right vectors in the x-z plane from a given rotation (calculated from yaw only)*/
void wrm_render_getOrientationXY(const vec3 rot, vec3 forward, vec3 right);

#endif
