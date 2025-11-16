#include "wrm/gui.h"
#include "wrm/memory.h"

# include <ft2build.h>
# include FT_FREETYPE_H


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

/*
Module internal constants
*/

#define WRM_GUI_INITIAL_ELEMENTS_CAPACITY 20

/*
Module internal globals
*/
extern FT_Library wrm_ft_library;
extern wrm_Stack wrm_fonts;

extern wrm_Pool wrm_gui_elements;
// extern wrm_Pool wrm_gui_child_lists; unused for now

/*
Module internal functions
*/

/* Draws text to the screen */
void wrm_gui_drawText(wrm_Text *t);
/* Draws a pane to the screen */
void wrm_gui_drawPane(wrm_Pane *p);