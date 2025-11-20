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
#include "wrm/linmath.h"
#include "SDL2/SDL.h"

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

/*
Type Declarations
*/

// render module settings
typedef struct wrm_render_Settings wrm_render_Settings;
// window creation arguments
typedef struct wrm_Window_Data wrm_Window_Data; 

// data format for meshes and shaders
typedef struct wrm_render_Format wrm_render_Format;
// simple shaders for various mesh types
typedef struct wrm_Default_Shaders wrm_Default_Shaders;

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

struct wrm_render_Settings {
    bool verbose;
    bool test;
    bool errors;
    char *shaders_dir;
};

struct wrm_Window_Data {
    const char *name; // the name of the window
    i32 height_px; // the height of the window in pixels
    i32 width_px; // the width of the window in pixels
    bool is_resizable; // whether or not the window should be resizable
    wrm_RGBA background; // RGBA background color (packed as a 32-bit unsigned integer)
};

struct wrm_render_Format {
    bool col;
    bool tex;
    u8 per_pos; // values per position, e.g. 3 for (x,y,z) coordinates, 2 for (x,y): shaders ALWAYS take position, and meshes MUST provide it
    // to add: normals, material properties, etc
};

struct wrm_Default_Shaders {
    wrm_Handle color; // default shader for a mesh with RGBA color attributes
    wrm_Handle texture; // default shader for a mesh with uv texture coordinates
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
    u32 channels; // number of channels of pixel data (should be 4, 3, or 1)
    bool transparent;
};

struct wrm_Mesh_Data {
    wrm_render_Format format; // data format of the mesh: must match shader used
    float *positions;       // position for each vertex
    float *colors;          // RGBA color for each vertex
    float *uvs;             // uv for each vertex
    u32 *indices;           // vertex indices
    size_t vtx_cnt;         // number of vertices for which we have data (independent of number of triangles)
    size_t idx_cnt;         // the number of indices in the mesh (3 * total tris for GL_TRIANGLES)
    u32 mode;
    bool cw;                // does the mesh use a clockwise winding order?
    bool dynamic;           // will we be frequently updating this mesh?
    bool transparent;       // if the mesh has colors, is the alpha anything other than 1 ?
};

struct wrm_Model_Data { 
    // these are relative to the parent model
    vec3 pos;
    vec3 rot;
    vec3 scale;

    wrm_Handle mesh;
    wrm_Handle texture; // only used when the model has a textured mesh; for now, meshes only use a single texture
    wrm_Handle shader;
    bool shown;
};

// default shaders - initialized within `render_init()`
extern wrm_Default_Shaders wrm_default_shaders;

/*
Module functions
*/

/* Initialize the renderer - opens a window with a GL context, sets up default shaders */
bool wrm_render_init(const wrm_render_Settings *settings, const wrm_Window_Data *data);
/* Shut down the renderer and clean up resources */
void wrm_render_quit(void);
/* Main drawing pass: renders all visible models to the framebuffer */
void wrm_render_draw(void);
/* Presents the next frame to the screen (separated from draw to allow for multiple passes over a frame) */
void wrm_render_present(void);
/* Get the window created by the render - may be needed by other modules */
SDL_Window *wrm_render_getWindow(void);
/* Updates renderer on a window resize event */
void wrm_render_onWindowResize(void);
/* Prints debug info about current render state to standard output */
void wrm_render_debugFrame(void);

// shader-related

/* Creates a shader program using the given shader source strings */
wrm_Option_Handle wrm_render_createShader(const char *vert, const char *frag, wrm_render_Format format);
/* For debugging; prints a shader's data to `stdout` */
void wrm_render_debugShader(wrm_Handle shader);
/* 
Removes a shader and its associated resources 
Called internally when shader creation fails and by wrm_render_quit() to free all render resources
As long as wrm_render_quit() is called this need not be
*/
void wrm_render_deleteShader(wrm_Handle shader);

// texture-related

/* Creates a texture */
wrm_Option_Handle wrm_render_createTexture(const wrm_Texture_Data *data);
/* Update part of a texture from the given x and y offset with the given data */
bool wrm_render_updateTexture(wrm_Handle texture, wrm_Texture_Data *data, u32 x, u32 y);
/* For debugging; prints a stexture's data to `stdout` */
void wrm_render_debugTexture(wrm_Handle texture);
/* 
Removes a texture and its associated resources 
Called internally when texture creation fails and by wrm_render_quit() to free all render resources
As long as wrm_render_quit() is called this need not be
*/
void wrm_render_deleteTexture(wrm_Handle texture);

// mesh-related

/* Create a mesh */
wrm_Option_Handle wrm_render_createMesh(const wrm_Mesh_Data *data);
/* Clones an existing mesh (useful for changing the mesh of a specific model without affecting others) */
wrm_Option_Handle wrm_render_cloneMesh(wrm_Handle mesh);
/* Updates a mesh's data: IMPORTANT: ALL models using this mesh will now use the updated version */
bool wrm_render_updateMesh(wrm_Handle mesh, const wrm_Mesh_Data *data);
/* For debugging; prints a mesh's data to `stdout` */
void wrm_render_debugMesh(wrm_Handle mesh);
/* 
Removes a mesh and its associated resources 
Called internally when mesh creation fails and by wrm_render_quit() to free all render resources
As long as wrm_render_quit() is called this need not be
*/
void wrm_render_deleteMesh(wrm_Handle mesh);

// model-related

/* Create a model - if use_default_shader is true, the renderer will attempt to select a default shader based on the mesh attributes */
wrm_Option_Handle wrm_render_createModel(const wrm_Model_Data *data, wrm_Handle *parent, bool use_default_shader);
/* Sets the given model's transform to the argument values, ignoring any NULL arguments */
bool wrm_render_setModelTransform(wrm_Handle model, const vec3 pos, const vec3 rot, const vec3 scale);
/* Adds the argument values to the given model's transform, ignoring any NULL arguments */
bool wrm_render_addModelTransform(wrm_Handle model, const vec3 pos, const vec3 rot, const vec3 scale);
/* Set a model's mesh */
bool wrm_render_setModelMesh(wrm_Handle model, wrm_Handle mesh);
/* Set a model's texture*/
bool wrm_render_setModelTexture(wrm_Handle model, wrm_Handle texture);
/* Set a model's shader - checks for compatibility with the model's mesh */
bool wrm_render_setModelShader(wrm_Handle model, wrm_Handle shader);
/* Toggle model visibility */
void wrm_render_setModelShown(wrm_Handle model, bool shown);
/* Toggle visibility of model's children */
void wrm_render_setChildrenShown(wrm_Handle model, bool shown);
/* Associates models `child` and `parent` as such */
bool wrm_render_addChild(wrm_Handle parent, wrm_Handle child);
/* Orphans model `child` from `parent` */
bool wrm_render_removeChild(wrm_Handle parent, wrm_Handle child);
/* creates a default colored test triangle - for testing */
wrm_Option_Handle wrm_render_createTestTriangle(void);
/* creates a default error-textured test cube - for testing */
wrm_Option_Handle wrm_render_createTestCube(void);
/* For debugging; prints a model's data to `stdout` */
void wrm_render_debugModel(wrm_Handle model);
/* 
Removes a model (BUT NOT its resources - these may be in use by other models)
Called internally when model creation fails and by wrm_render_quit() to free all render resources
As long as wrm_render_quit() is called this need not be
*/
void wrm_render_deleteModel(wrm_Handle model);

// camera-related

/* unusable at the moment; might be used later for projects where multiple cameras may be required */
// wrm_Option_Handle wrm_render_createCamera();
/* Updates the viewing camera; ignores any NULL values */
void wrm_render_updateCamera(float *fov, float *offset, const vec3 pos, const vec3 rot);
/* Gets the render's camera data, stores results in the provided pointers */
void wrm_render_getCameraData(float *fov, float *offset, vec3 pos, vec3 rot);
/* print debug information about the camera */
void wrm_render_debugCamera(void);



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

/* get forward, up, and right vectors from a given orientation vector: applies yaw->pitch->roll*/
void wrm_render_getOrientation(const vec3 rot, vec3 forward, vec3 up, vec3 right);
/* gets forward and right vectors in the x-z plane from a given rotation (calculated from yaw only)*/
void wrm_render_getOrientationXY(const vec3 rot, vec3 forward, vec3 right);

#endif
