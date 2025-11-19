#include "wrm/common.h"
#include "wrm/render.h"
#include "wrm/memory.h"
#include "wrm/linmath.h"
#include "stb/stb_image.h"
#include "glad/glad.h"

/*
For wrm-render

Every wrm-render
Constants are actually defined in render-constants.c

Globals are defined/managed in their render.c
*/


/*
Internal type definitions
*/

// shader GL data, plus requirements of meshes rendered with it
typedef struct wrm_Shader {
    wrm_render_Format format;
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
    wrm_render_Format format;
    GLuint vao;
    GLuint pos_vbo;
    GLuint uv_vbo;
    GLuint col_vbo;
    GLuint ebo;
    size_t count;
    GLenum mode;
    bool cw;
} wrm_Mesh;

typedef struct wrm_Model {
    vec3 pos;
    vec3 rot;
    vec3 scale;

    wrm_Handle mesh;
    wrm_Handle texture; // only used when the model has a textured mesh; for now, meshes only use a single texture
    wrm_Handle shader;

    wrm_Tree_Node tree_node; // tree node for model hierarchy
    bool show_children;
    bool is_visible;
} wrm_Model;

// camera data
typedef struct wrm_Camera {
    float offset; // distance forward (positive) or backward (negative) along `facing` from `pos`: used for 3rd-person controls
    float fov;
    vec3 pos;
    vec3 rot;
} wrm_Camera;


DEFINE_OPTION(GLuint, GLuint);

DEFINE_LIST(wrm_Model_Data, Model_Data);

// resource enumeration
typedef enum wrm_render_Resource_Type {
    WRM_RENDER_RESOURCE_MODEL, 
    WRM_RENDER_RESOURCE_MESH, 
    WRM_RENDER_RESOURCE_SHADER, 
    WRM_RENDER_RESOURCE_TEXTURE
} wrm_render_Resource_Type;

/*
Constants
*/

// windowing constants

extern const u32 WRM_DEFAULT_WINDOW_HEIGHT;
extern const u32 WRM_DEFAULT_WINDOW_WIDTH;

extern const u32 WRM_MESH_TRIANGLE;
extern const u32 WRM_MESH_STRIP;
extern const u32 WRM_MESH_FAN;

// general rendering constants

extern const float WRM_NEAR_CLIP_DISTANCE;
extern const float WRM_FAR_CLIP_DISTANCE;

extern const u32 WRM_SHADER_ATTRIB_POS_LOC;
extern const u32 WRM_SHADER_ATTRIB_COL_LOC;
extern const u32 WRM_SHADER_ATTRIB_UV_LOC;
/* extern const u32 WRM_SHADER_ATTRIB_NORM_LOC = 3; unused (yet) */

// shader for color

extern const char *WRM_SHADER_DEFAULT_COL_V_TEXT;
extern const char *WRM_SHADER_DEFAULT_COL_F_TEXT;

// shader for texture

extern const char *WRM_SHADER_DEFAULT_TEX_V_TEXT;
extern const char *WRM_SHADER_DEFAULT_TEX_F_TEXT;

// shader for both

extern const char *WRM_SHADER_DEFAULT_BOTH_V_TEXT;
extern const char *WRM_SHADER_DEFAULT_BOTH_F_TEXT;

// default meshes

extern const wrm_Mesh_Data default_meshes_colored_triangle;
extern const wrm_Mesh_Data default_meshes_colored_cube;
extern const wrm_Mesh_Data default_meshes_textured_cube;

// pool constants

extern const u32 WRM_RENDER_POOL_INITIAL_CAPACITY;

// list constants

extern const u32 WRM_RENDER_LIST_INITIAL_CAPACITY;
extern const u32 WRM_RENDER_LIST_SCALE_FACTOR;

/*
Module-level globals
*/

extern u32 wrm_ui_count;

extern wrm_Pool wrm_shaders;
extern wrm_Pool wrm_meshes;
extern wrm_Pool wrm_textures;
extern wrm_Pool wrm_models;

extern wrm_Tree wrm_model_tree;

extern wrm_Camera wrm_camera;

extern wrm_render_Settings wrm_render_settings;
extern bool wrm_render_is_initialized;

extern int wrm_window_width;
extern int wrm_window_height;

extern bool wrm_render_debug_frame;

extern wrm_Camera wrm_camera;

/*
Module internal functions
*/

// compiles a shader from the given shader text
wrm_Option_GLuint wrm_render_compileShader(const char *shader_text, GLenum type);
// creates a default shader for meshes with per-vertex colors, per-vertex uv's, and both
bool wrm_render_createDefaultShaders(const char *shader_dir);
// loads a shader .frag and .vert pair with the given name, from the given directory, with the given format
wrm_Option_Handle wrm_render_loadAndCreateShader(const char *dir, const char *name, wrm_render_Format format);
// creates a default pink-and-black error texture
bool wrm_render_createErrorTexture(void);
// checks whether resource handle `h` to a resource of type `t` is in use 
// `caller` should be the name of the calling function, and `context` should explain the context of the check for helpful error messages
bool wrm_render_exists(wrm_Handle h, wrm_render_Resource_Type t, const char *caller, const char *context);
// gets the default shader for a given mesh
wrm_Option_Handle wrm_render_getDefaultShader(wrm_Handle mesh);
// gets the view matrix from the current camera orientation
void wrm_render_getViewMatrix(mat4 view);
/*
creates a GL vertex buffer object (VBO)
`attr_loc` is the `layout(location=<n>)` location in the vertex shader that this vbo holds data for
`num_entries` is the total number of sets of data (e.g. position vectors, colors, etc)
`values_per_entry` is how many (assumed to be float) values there are for each entry (2 for vec2, 3 for vec3, etc)
*/
void wrm_render_createVBO(GLuint *vbo, u32 attr_loc, size_t num_entries, size_t values_per_entry, const void *data, GLenum usage);

/* Inline functions to Set GL state */

// Sets the GL shader program to the given WRM shader, if valid 
inline void wrm_render_setGLShader(wrm_Handle shader)
{
    wrm_Shader *s = wrm_Pool_at(&wrm_shaders, shader);
    if(s) glUseProgram(s->program);
}
// sets the GL texture to the given WRM texture, if valid
inline void wrm_render_setGLTexture(wrm_Handle texture)
{
    wrm_Texture *t = wrm_Pool_at(&wrm_textures, texture);
    if(t) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, t->gl_tex);
    }
}

// internal cleanup functions (NOT user visible, use pointers)
void wrm_Shader_delete(void *shader);
void wrm_Mesh_delete(void *mesh);
void wrm_Texture_delete(void *texture);
void wrm_Model_delete(void *model);

