#include "wrm/menu.h"
#include "wrm/memory.h"

#include <ft2build.h>
#include FT_FREETYPE_H


/*
Internal type declarations
*/

typedef struct wrm_Font wrm_Font;

typedef struct wrm_Glyph wrm_Glyph;
typedef struct wrm_Mesh_Buffer wrm_Mesh_Buffer;

typedef struct wrm_Vertex wrm_Vertex;
typedef struct wrm_Quad wrm_Quad;

/*
Internal type definitions
*/

struct wrm_Glyph {
    float advance_x; // advance.x
    float advance_y; // advance.y
  
    float bmp_w; // bitmap.width
    float bmp_h; // bitmap.rows
  
    float bmp_left; // bitmap_left
    float bmp_top; // bitmap_top
  
    float tex_offset; // x offset of glyph in texture coordinates
};

struct wrm_Font {
    wrm_Handle atlas;
    wrm_Glyph *glyph_set;
    FT_Face face;
};

struct wrm_Mesh_Buffer {
    float *positions;
    float *colors;
    float *uvs;
    u32 *indices;
    u32 vtx_len;
    u32 vtx_cap;
    u32 idx_len;
    u32 idx_cap;
};

struct wrm_Vertex {
    vec3 pos;
    vec2 uv;
    vec4 col;
};

struct wrm_Quad {
    wrm_Vertex tl;
    wrm_Vertex tr;
    wrm_Vertex bl;
    wrm_Vertex br;
    bool has_col;
    bool has_uv;
};


/* 
Constants
*/

extern const u8 WRM_FONT_GLYPHS_COUNT;

/*
Globals
*/

// window parameters
extern int wrm_window_width;
extern int wrm_window_height;

// vertical scale of a single pixel in Normalized Device Coordinates
extern float ndc_scale_h;
// horizontal scale of a pixel in Normalized Device Coordinates
extern float ndc_scale_w;


extern wrm_Settings wrm_menu_settings;

// menu elements
extern wrm_Pool wrm_elements; // pool of element data

// text rendering
extern wrm_Stack wrm_fonts;
extern FT_Library wrm_ft_library;

extern wrm_Mesh_Buffer wrm_mesh_buf;

/*
module internal helper declarations
*/

/* Sets the positions of a quad based on the top-left coordinate, width, and height */
void wrm_menu_setQuadPositions(wrm_Quad *q, vec2 tl, float width, float height);
/* Sets the colors of a quad at each corner */
void wrm_menu_setQuadColors(wrm_Quad *q, wrm_RGBAf tl, wrm_RGBAf tr, wrm_RGBAf bl, wrm_RGBAf br);
/* Sets the uvs of a quad at each corner based on the uv of the top-left and the width and height (in texture coordinates) */
void wrm_menu_setQuadUVs(wrm_Quad *q, vec2 tl, float width, float height);
/* Resets the mesh buffer */
inline void wrm_menu_resetMeshBuffer(wrm_Mesh_Buffer *m)
{
    if(!m) return;
    m->vtx_len = 0;
    m->idx_len = 0;
}
/* adds a vertex to the mesh buffer */
bool wrm_menu_addVertex(wrm_Mesh_Buffer *m, wrm_Vertex *v, bool colors, bool uvs);
/* adds a list of vertex indices to the mesh buffer */
bool wrm_menu_addIndices(wrm_Mesh_Buffer *m, u32* indices, u8 count);
/* adds a quad to the mesh buffer */
bool wrm_menu_addQuad(wrm_Mesh_Buffer *m, wrm_Quad *q);
/* Creates the required data from the rendering module for a given menu element */
wrm_Option_Handle wrm_menu_createRenderData(wrm_Menu_Element *e);
/* Checks whether a given menu element actually exists */
bool wrm_menu_exists(wrm_Handle h, const char *caller, const char *type, const char *reason);
/* Create a single menu element to test the system */
void wrm_menu_createTestElement(void);