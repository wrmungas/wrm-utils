#include "wrm/gui.h"
#include "wrm/memory.h"
// access some render module internals and GL functions
#include "../render/render.h"

# include <ft2build.h>
# include FT_FREETYPE_H


/*
Internal type declarations
*/

typedef struct wrm_Font wrm_Font;

typedef struct wrm_Glyph wrm_Glyph;

typedef struct wrm_Mesh_Buffer wrm_Mesh_Buffer;

// represents four vertices
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
    u32 atlas_height; // the height of the atlas texture in pixels
    wrm_Glyph *glyph_set;
    FT_Face face;
};


struct wrm_Quad {
    float positions[2*4];
    float colors[4*4];
    float uvs[2*4];

    GLuint vao;
    GLuint pos_vbo;
    GLuint col_vbo;
    GLuint uv_vbo;
    GLuint ebo;
};

/*
Module internal constants
*/

#define WRM_GUI_INITIAL_ELEMENTS_CAPACITY 20

#define WRM_QUAD_TL 0
#define WRM_QUAD_TR 1
#define WRM_QUAD_BL 2
#define WRM_QUAD_BR 3

#define WRM_QUAD_TOTAL 4

/*
Module internal globals
*/
extern FT_Library wrm_ft_library;
extern wrm_Stack wrm_fonts;

extern wrm_Pool wrm_gui_elements;
extern wrm_Quad the_quad;

extern wrm_Handle wrm_gui_text_shader;
extern wrm_Handle wrm_gui_image_shader;
extern wrm_Handle wrm_gui_pane_shader;

// extern wrm_Pool wrm_gui_child_lists; unused for now

/*
Module internal functions
*/

// general

inline bool wrm_gui_exists(wrm_Handle e)
{
    return e < wrm_gui_elements.cap && wrm_gui_elements.is_used[e];
}

// alignment

/* Gets floating point NDC from screen pixel coordinates */
inline void wrm_gui_NDC_fromPos(i32 x, i32 y, vec2 dest)
{
    dest[WRM_X] = 2.0f * (float)x / (float)wrm_window_width - 1.0f;
    dest[WRM_Y] = -2.0f * (float)y / (float)wrm_window_height + 1.0f; // flip y-axis (pixel y coord increases downward, NDC y coord increases upward)
}
/* Gets floating point NDC dimensions from screen pixel dimensions */
inline void wrm_gui_NDC_fromDim(i32 w, i32 h, vec2 dest)
{
    dest[WRM_X] = 2.0f * (float)w / (float)wrm_window_width;
    dest[WRM_Y] = 2.0f * (float)h / (float)wrm_window_height;
}
/* Gets top-left position and dimensions of an element in NDC from the element's alignment */
void wrm_gui_NDC_fromAlignment(wrm_gui_Alignment a, vec2 pos, vec2 dim);
/* gets the top-left point (in screen pixels) of an alignment struct */
void wrm_gui_getTopLeft(wrm_gui_Alignment a, i32 *tl_x, i32 *tl_y);


// text

/* Draws text to the screen */
void wrm_gui_drawText(wrm_Text *t);

// pane

/* Draws a pane to the screen */
void wrm_gui_drawPane(wrm_Pane *p);

// image

/* Draws an image to the screen */
void wrm_gui_drawImage(wrm_Image *i);

// quad functions

/* Create The Quad's GPU data */
void wrm_gui_initQuad(void);
/* Update The Quad's GPU data */
void wrm_gui_updateQuad(void);
/* (sniff) destroy The Quad's GPU data */
void wrm_gui_deleteQuad(void);
/* Set the position of the given vertex in The Quad */
inline void wrm_gui_setQuadPos(int vtx, const vec2 pos)
{
    if(vtx > 3) return;

    the_quad.positions[vtx * 2 + 0] = pos[WRM_X];
    the_quad.positions[vtx * 2 + 1] = pos[WRM_Y];
}
/* Set the color of the given vertex in The Quad */
inline void wrm_gui_setQuadCol(int vtx, const wrm_RGBAf col)
{
    if(vtx > 3) return;

    the_quad.positions[vtx * 4 + 0] = col.r;
    the_quad.positions[vtx * 4 + 0] = col.g;
    the_quad.positions[vtx * 4 + 0] = col.b;
    the_quad.positions[vtx * 4 + 0] = col.a;
}
/* Set the uv coordinates of the given vertex in The Quad */
inline void wrm_gui_setQuadUV(int vtx, const vec2 uv)
{
    if(vtx > 3) return;
    the_quad.uvs[vtx * 2 + 0] = uv[WRM_X];
    the_quad.uvs[vtx * 2 + 1] = uv[WRM_Y];
}
/* Set the vertex data of the given vertex in The Quad */
inline void wrm_gui_setQuadVert(int vtx, const vec2 pos, const wrm_RGBAf col, const vec2 uv)
{
    if(vtx > 3) return;
    wrm_gui_setQuadPos(vtx, pos);
    wrm_gui_setQuadCol(vtx, col);
    wrm_gui_setQuadUV(vtx, uv);
}
/* Sets the position of all four vertices based on the top left position plus width and height dimensions*/
inline void wrm_gui_setQuadCorners(const vec2 tl, const vec2 dim)
{
    wrm_gui_setQuadPos(WRM_QUAD_TL, tl);
    wrm_gui_setQuadPos(WRM_QUAD_TR, (vec2){tl[WRM_X] + dim[WRM_X], tl[WRM_Y]});
    wrm_gui_setQuadPos(WRM_QUAD_BL, (vec2){tl[WRM_X], tl[WRM_Y] - dim[WRM_Y]});
    wrm_gui_setQuadPos(WRM_QUAD_BL, (vec2){tl[WRM_X] + dim[WRM_X], tl[WRM_Y] - dim[WRM_Y]});
}