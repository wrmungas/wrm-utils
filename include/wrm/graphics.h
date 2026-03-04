#ifndef WRM_GFX_H
#define WRM_GFX_H
/* --- HEADER DESCRIPTION -----------------------------------------------------
File gl.h

Created Oct 29, 2025 
by William R Mungas (wrm)

Version: 0.1.0 
(Last modified Mar 3, 2026)

DESCRIPTION:
Rendering framework that uses SDL to create a window, which is exposed 
externally for wrm-input. Currently uses cglm as a vector/matrix 
gl-optimized math library.

In the future, I may abstract general window functionality to a separate 
header called wrm-window or something similar, so that this and wrm-input 
are less tightly coupled. I may also provide a rendering backend based on 
Vulkan instead of OpenGL.

PROVIDES:
- rendering primitives: shader, texture, mesh, model
- access to a default camera

REQUIREMENTS:
- SDL2 must be installed for headers/linking
- cglm/ must also be in your project's includes, used as header-only lib

---------------------------------------------------------------------------- */

#include "wrm/common.h"
#include "wrm/linmath.h"
#include "SDL2/SDL.h"

/* --- Type Declarations --------------------------------------------------- */

// render module settings
typedef struct gfx_Settings 
gfx_Settings;
// window creation arguments
typedef struct gfx_Window_Info 
gfx_Window_Info; 
// data format shared between meshes and shaders
typedef struct gfx_Format 
gfx_Format;
// single 32-bit integer rgba value
typedef u32 rgba32;
// struct of 4 bytes: r, g, b, a
typedef struct gfx_RGBA 
gfx_RGBA;
// struct of 4 floats: r, g, b, a: used for when OpenGL wants color channel 
// values as floats in the range [0.0f, 1.0f]
typedef struct gfx_RGBAf 
gfx_RGBAf;
// data for creating a shader
typedef struct gfx_Shader_Info 
gfx_Shader_Info;
// data for creating a texture
typedef struct gfx_Texture_Info 
gfx_Texture_Info;
// Arguments for mesh creation
typedef struct wgfx_Mesh_Info 
gfx_Mesh_Info;
// Arguments for model creation
typedef struct wrm_Model_Info 
wrm_Model_Info;

/* --- Type definitions ---------------------------------------------------- */

struct wrm_Window_Info {
    const char *name; // the name of the window
    i32 height_px; // the height of the window in pixels
    i32 width_px; // the width of the window in pixels
    bool is_resizable; // whether or not the window should be resizable
    wrm_gfx_RGBAi background; // RGBA background color
};

struct wrm_gfx_Format { // TODO add normals, material properties, etc
    bool col;
    bool tex;
    u8 per_pos; // values per position, 
    // e.g. 3 for (x,y,z) coordinates, 2 for (x,y)
    // shaders ALWAYS take position, and meshes MUST provide it
};

struct wrm_RGBA {
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

struct wrm_Shader_Info {
    wrm_GL_Format format;
    char *vert;
    char *frag; 
};

struct wrm_Texture_Info {
    u8 *pixels; // array of 4 x width * height bytes (or NULL to fill later)
    u32 width; // width of the texture, in pixels
    u32 height; // height of the texture, in pixels
    u32 channels; // number of channels of pixel data (should be 4, 3, or 1)
    bool transparent;
};

struct wrm_Mesh_Info {
    wrm_gfx_Format format; // data format of the mesh: must match shader used
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

struct wrm_Model_Info { 
    // these are relative to the parent model
    vec3 pos;
    vec3 rot;
    vec3 scale;

    wrm_Ref mesh;
    wrm_Ref texture; // only used when the model has a textured mesh; for now, meshes only use a single texture
    wrm_Ref shader;
    bool shown;
};

/* --- Externally visible constants ---------------------------------------- */

extern const u32 WRM_MESH_TRIANGLE;
extern const u32 WRM_MESH_STRIP;
extern const u32 WRM_MESH_FAN;

extern const u32 WRM_DEFAULT_WINDOW_WIDTH;
extern const u32 WRM_DEFAULT_WINDOW_HEIGHT;

#define WRM_MODEL_CHILD_LIMIT 4

// bit masks for colors 

#define WRM_RGBA32_R_BITS 0xff000000u
#define WRM_RGBA32_G_BITS 0x00ff0000u
#define WRM_RGBA32_B_BITS 0x0000ff00u
#define WRM_RGBA32_A_BITS 0x000000ffu

// a few simple predefined colors

#define WRM_RGBA32_BLACK 0x000000ffu
#define WRM_RGBA32_WHITE 0xffffffffu

#define WRM_RGBA32_RED 0xff0000ffu
#define WRM_RGBA32_GREEN 0x00ff00ffu
#define WRM_RGBA32_BLUE 0x0000ffffu

#define WRM_RGBA32_PURPLE 0xff00ffffu
#define WRM_RGBA32_YELLOW 0xffff00ffu
#define WRM_RGBA32_CYAN 0x00ffffffu

/* --- Module functions ---------------------------------------------------- */

// --- GRAPHICS MODULE ---

/* 
Initialize the renderer - opens a window with a GL context, 
sets up default shaders 
*/
bool wrm_gfx_init(
    const wrm_gfx_Settings *settings, 
    const wrm_gfx_Window_Info *winargs
);
/* Shut down the renderer and clean up resources */
void wrm_gfx_quit(void);
/* Main drawing pass: renders all visible models to the framebuffer */
void wrm_gfx_draw(void);
/* 
Presents the next frame to the screen (separated from draw to allow for 
multiple passes over a frame) 
*/
void wrm_gfx_present(void);
/* Get the window created by the render - may be needed by other modules */
SDL_Window *wrm_gfx_getWindow(void);
/* Updates renderer on a window resize event */
void wrm_gfx_onWindowResize(void);
/* Prints debug info about current render state to standard output */
void wrm_gfx_debugFrame(void);

// --- SHADER ---

/* Creates a shader program using the given shader source strings */
bool wrm_createShader( 
    wrm_gfx_Shader_Info *args,
    wrm_Ref *dest
);
/* For debugging; prints a shader's data to `stdout` */
void wrm_debugShader(wrm_Ref shader);
/* 
Removes a shader and its associated resources 
Called internally when shader creation fails and by wrm_gfx_quit() to free all 
render resources
As long as wrm_gfx_quit() is called this need not be
*/
void wrm_deleteShader(wrm_Ref shader);

// --- TEXTURE ---

/* Creates a texture */
bool wrm_createTexture(const wrm_gfx_Texture_Info *args, wrm_Ref *dest);
/* 
Update the data of texture, offset from x and y, with the provided pixel
data 
*/
bool wrm_updateTexture(
    wrm_Ref texture, 
    wrm_gfx_Texture_Info *data, 
    u32 x, 
    u32 y
);
/* For debugging; prints a stexture's data to `stdout` */
void wrm_debugTexture(wrm_Ref texture);
/* 
Removes a texture and its associated resources 
Called internally when texture creation fails and by wrm_gfx_quit() to free all
render resources
As long as wrm_gfx_quit() is called this need not be
*/
void wrm_deleteTexture(wrm_Ref texture);

// --- MESH ---

/* Create a mesh */
bool wrm_createMesh(const wrm_gfx_Mesh_Info *args, wrm_Ref *dest);
/* 
Clones an existing mesh (useful for changing the mesh of a single entity using
the model without affecting all others
*/
bool wrm_cloneMesh(wrm_Ref src, wrm_Ref *dest);
/* 
Updates a mesh's data: IMPORTANT: ALL models using this mesh will now use 
the updated version 
*/
bool wrm_updateMesh(wrm_Ref mesh, const wrm_gfx_Mesh_Info *data);
/* For debugging; prints a mesh's data to `stdout` */
void wrm_debugMesh(wrm_Ref mesh);
/* 
Removes a mesh and its associated resources 
Called internally when mesh creation fails and by wrm_gfx_quit() to free all 
render resources
As long as wrm_gfx_quit() is called this need not be called directly
*/
void wrm_deleteMesh(wrm_Ref mesh);

// --- MODEL ---

/* 
Create a model - if use_default_shader is true, the renderer will attempt to 
select a default shader based on the mesh attributes 
*/
bool wrm_createModel(
    const wrm_gfx_Model_Info *data, 
    wrm_Ref *parent, 
    bool use_default_shader
);
/* 
Sets the given model's transform to the argument values
Ignores any NULL arguments 
*/
bool wrm_setModelTransform(
    wrm_Ref model, 
    const vec3 pos, 
    const vec3 rot, 
    const vec3 scale
);
/* 
Adds the argument values to the given model's transform, 
ignoring any NULL arguments 
*/
bool wrm_addModelTransform(
    wrm_Ref model, 
    const vec3 pos, 
    const vec3 rot, 
    const vec3 scale
);
/* Set a model's mesh */
bool wrm_setModelMesh(wrm_Ref model, wrm_Ref mesh);
/* Set a model's texture*/
bool wrm_setModelTexture(wrm_Ref model, wrm_Ref texture);
/* Set a model's shader - checks for compatibility with the model's mesh */
bool wrm_setModelShader(wrm_Ref model, wrm_Ref shader);
/* Toggle model visibility */
void wrm_setModelShown(wrm_Ref model, bool shown);
/* Toggle visibility of model's children */
void wrm_setChildrenShown(wrm_Ref model, bool shown);
/* Associates models `child` and `parent` as such */
bool wrm_addChild(wrm_Ref parent, wrm_Ref child);
/* Orphans model `child` from `parent` */
bool wrm_removeChild(wrm_Ref parent, wrm_Ref child);
/* creates a default colored test triangle - for testing */
bool wrm_createTestTriangle(wrm_Ref *dest);
/* creates a default error-textured test cube - for testing */
bool wrm_createTestCube(wrm_Ref *dest);
/* For debugging; prints a model's data to `stdout` */
void wrm_debugModel(wrm_Ref model);
/* 
Removes a model (BUT NOT its resources - these may be in use by other models)
Called internally when model creation fails and by wrm_gfx_quit() to free all 
render resources
As long as wrm_gfx_quit() is called this need not be
*/
void wrm_gfx_deleteModel(wrm_Ref model);

// --- CAMERA ---

/* 
unusable at the moment; might be used later for projects where multiple cameras
may be required 
*/
// wrm_Option_Handle wrm_gfx_createCamera();
/* Updates the viewing camera; ignores any NULL values */
void wrm_gfx_updateCamera(
    float *fov, 
    float *offset, 
    const vec3 pos, 
    const vec3 rot
);
/* Gets the render's camera data, stores results in the provided pointers */
void wrm_gfx_getCameraData(float *fov, float *offset, vec3 pos, vec3 rot);
/* print debug information about the camera */
void wrm_gfx_debugCamera(void);



/* Self-explanatory RGBA functions: inlined because they are very small */

inline wrm_gfx_RGBAi wrm_RGBAi_fromRGBA(wrm_gfx_RGBA rgba) 
{
    return (u32)rgba.r << 24 
    | (u32)rgba.g << 16 
    | (u32)rgba.b << 8 
    | (u32)rgba.a;
}

inline wrm_gfx_RGBAi wrm_RGBAi_fromRGBAf(wrm_gfx_RGBAf rgbaf)
{
    return (u32)(rgbaf.r * 255.0) << 24 
    | (u32)(rgbaf.g * 255.0) << 24 
    | (u32)(rgbaf.b * 255.0) << 24 
    | (u32)(rgbaf.a * 255.0);
}

inline wrm_gfx_RGBA wrm_RGBA_fromRGBAi(wrm_gfx_RGBAi rgbai)
{
    return (wrm_gfx_RGBA){
        .r = (u8)((rgbai & WRM_RGBAi_R_BITS) >> 24),
        .g = (u8)((rgbai & WRM_RGBAi_G_BITS) >> 16),
        .b = (u8)((rgbai & WRM_RGBAi_B_BITS) >> 8),
        .a = (u8)(rgbai & WRM_RGBAi_A_BITS)
    };
}

inline wrm_gfx_RGBA wrm_RGBA_fromRGBAf(wrm_gfx_RGBAf rgbaf)
{
    return (wrm_gfx_RGBA) {
        .r = (u8)(rgbaf.r * 255.0f),
        .g = (u8)(rgbaf.g * 255.0f),
        .b = (u8)(rgbaf.b * 255.0f),
        .a = (u8)(rgbaf.a * 255.0f),
    };
}

inline wrm_gfx_RGBAf wrm_RGBAf_fromRGBAi(wrm_gfx_RGBAi rgbai)
{
    return (wrm_gfx_RGBAf) {
        .r = (float)((rgbai & WRM_RGBAi_R_BITS) >> 24) / 255.0f,
        .g = (float)((rgbai & WRM_RGBAi_G_BITS) >> 16) / 255.0f,
        .b = (float)((rgbai & WRM_RGBAi_B_BITS) >> 8) / 255.0f,
        .a = (float)(rgbai & WRM_RGBAi_A_BITS) / 255.0f
    };
}

inline wrm_gfx_RGBAf wrm_RGBAf_fromRGBA(wrm_gfx_RGBA rgba)
{
    return (wrm_gfx_RGBAf) {
        .r = (float)(rgba.r) / 255.0f,
        .g = (float)(rgba.g) / 255.0f,
        .b = (float)(rgba.b) / 255.0f,
        .a = (float)(rgba.a) / 255.0f
    };
}

// vector functions

/* 
get forward, up, and right vectors from a given orientation vector.
applies yaw->pitch->roll
*/
void wrm_gfx_getOrientation(const vec3 rot, vec3 forward, vec3 up, vec3 right);
/* 
gets forward and right vectors in the x-z plane from a given rotation 
(calculated from yaw only)
*/
void wrm_gfx_getOrientationXY(const vec3 rot, vec3 forward, vec3 right);

#endif // end include guards