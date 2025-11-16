#ifndef WRM_MENU_H
#define WRM_MENU_H

/*

File wrm-common.h

Created Nov 7, 2025
by William R Mungas (wrm)

Version: 0.1.0
(Last modified Nov 7, 2025)

DESCRIPTION:
Provides some simple ui/menu primitives, to be used with
wrm-render and wrm-input

PROVIDES
- basic UI types
- text renderin

REQUIREMENTS:
- FreeType library must be installed/linked with
*/

# ifdef WRM_MENU_IMPLEMENTATION_H
    #include <ft2build.h> 
    #include FT_FREETYPE_H
# endif

#include "wrm/common.h"
#include "wrm/render.h"
#include "wrm/input.h"

/*
Type declarations
*/

/* Represents a colored or textured pane as a background for other elements */
typedef struct wrm_Pane wrm_Pane;
/* Represents a box of text; should probably have a semi- or fully transparent parent pane */
typedef struct wrm_Text_Box wrm_Text_Box;
/* Represents a button element */
typedef struct wrm_Button wrm_Button;
/* Represents a slider element */
typedef struct wrm_Slider wrm_Slider;
/* Generic element: includes general element data and data specific to the element type */
typedef struct wrm_Menu_Element wrm_Menu_Element;
/* Union of all element types */
typedef union wrm_Menu_Element_Any wrm_Menu_Element_Any;

/*
Type definitions
*/

struct wrm_Text_Box {
    wrm_Handle font;
    u32 text_len;
    void (*on_click)(void *); // click callback
    const char *text;
};

struct wrm_Pane {
    bool moveable;
};

union wrm_Menu_Element_Any {
    wrm_Text_Box text_box;
};

struct wrm_Menu_Element {
    enum {WRM_TEXT_BOX, WRM_PANE, WRM_BUTTON, WRM_SLIDER, WRM_TOGGLE} type;
    wrm_Menu_Element_Any self;
    float x; // x position of top left corner on screen, in NDC
    float y; // y position of top left corner on screen, in NDC
    u32 width; // width in pixels
    u32 height; // height in pixels
    u32 model;
    u32 parent;
    u32 children[WRM_MODEL_CHILD_LIMIT];
    u8 child_count;
    bool visible;
    bool show_children;
};


/* 
Constants
*/

extern wrm_Handle WRM_MENU_NONE;

/*
Globals
*/

/*
Function declarations
*/

// module

/* Initialize the menuing system */
bool wrm_menu_init();
/* Update menu state based on user input */
void wrm_menu_update(void);
/* Shut down the menu system */
void wrm_menu_quit(void);

// font

/* Load a font from a given font file */
wrm_Option_Handle wrm_menu_loadFont(const char *path);

// text box

/* Create a text box menu element */
wrm_Option_Handle wrm_menu_createTextBox(wrm_Text_Box *t , float x, float y, u32 width, u32 height, wrm_Handle parent);
/* Update a text box menu element */
void wrm_menu_updateTextBox(wrm_Handle textbox, wrm_Text_Box *t);

// pane

/* Create a pane element */
wrm_Option_Handle wrm_menu_createPane(wrm_Pane *p, u32 x, u32 y, u32 width, u32 height, wrm_Handle parent);

// general elements

/* Add one menu element to another as a child */
void wrm_menu_addChild(wrm_Handle parent, wrm_Handle child);
/* Remove the relationship between two menu elements, orphaning the child */
void wrm_menu_removeChild(wrm_Handle parent, wrm_Handle child);

# ifdef WRM_MENU_IMPLEMENTATION

#include "wrm/memory.h"

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



DEFINE_LIST(wrm_Font, Font);
DEFINE_LIST(wrm_Menu_Element, Menu_Element);

/* 
Constants
*/

wrm_Handle WRM_MENU_NONE = 0;
internal const u8 WRM_FONT_GLYPHS_COUNT = 128 - 32;

/*
Globals
*/

// window parameters
internal int wrm_window_width;
internal int wrm_window_height;

// vertical scale of a single pixel in Normalized Device Coordinates
internal float ndc_scale_h;
// horizontal scale of a pixel in Normalized Device Coordinates
internal float ndc_scale_w;


internal wrm_Settings wrm_menu_settings;

// menu elements

internal wrm_Pool wrm_elements; // pool of element data

// text rendering

internal wrm_List_Font wrm_fonts;
internal FT_Library wrm_ft_library;
internal wrm_Mesh_Buffer wrm_mesh_buf;



/*
Internal helper declarations
*/

/* Sets the positions of a quad based on the top-left coordinate, width, and height */
internal inline void wrm_menu_setQuadPositions(wrm_Quad *q, vec2 tl, float width, float height);
/* Sets the colors of a quad at each corner */
internal inline void wrm_menu_setQuadColors(wrm_Quad *q, wrm_RGBAf tl, wrm_RGBAf tr, wrm_RGBAf bl, wrm_RGBAf br);
/* Sets the uvs of a quad at each corner based on the uv of the top-left and the width and height (in texture coordinates) */
internal inline void wrm_menu_setQuadUVs(wrm_Quad *q, vec2 tl, float width, float height);
/* Resets the mesh buffer */
internal inline void wrm_menu_resetMeshBuffer(wrm_Mesh_Buffer *m);
/* adds a vertex to the mesh buffer */
internal inline bool wrm_menu_addVertex(wrm_Mesh_Buffer *m, wrm_Vertex *v, bool colors, bool uvs);
/* adds a list of vertex indices to the mesh buffer */
internal inline bool wrm_menu_addIndices(wrm_Mesh_Buffer *m, u32* indices, u8 count);
/* adds a quad to the mesh buffer */
internal inline bool wrm_menu_addQuad(wrm_Mesh_Buffer *m, wrm_Quad *q);
/* Creates the required data from the rendering module for a given menu element */
internal wrm_Option_Handle wrm_menu_createRenderData(wrm_Menu_Element *e);
/* Updates the mesh of a text box to include quads for every character in the text */
internal wrm_Option_Handle wrm_menu_updateTextBoxMesh(wrm_Menu_Element *tb);
/* Checks whether a given menu element actually exists */
internal bool wrm_menu_exists(wrm_Handle h, const char *caller, const char *type, const char *reason);
/* Create a single menu element to test the system */
internal void wrm_menu_createTestElement(void);


/*
Module function definitions
*/

bool wrm_menu_init(wrm_Settings *s)
{
    wrm_menu_settings = *s;
    
    wrm_Pool_init(&wrm_elements, 10, sizeof(wrm_Menu_Element));

    wrm_fonts = (wrm_List_Font) {
        .cap = 3,
        .len = 0,
        .data = calloc(3, sizeof(wrm_Font))
    };
    wrm_mesh_buf = (wrm_Mesh_Buffer) {
        .vtx_cap = 100,
        .vtx_len = 0,
        .positions = calloc(100 * 3, sizeof(float)),
        .uvs = calloc(100 * 2, sizeof(float)),
        .colors = calloc(100 * 4, sizeof(float)),
        .idx_cap = 50,
        .idx_len = 0,
        .indices = calloc(50, sizeof(u32)),
    };

    int error = FT_Init_FreeType(&wrm_ft_library);
    if(error) {
        if(wrm_menu_settings.errors) { wrm_error("Menu: init()", "failed to initialize FreeType"); }
        return false;
    }

    // reserve element 0 as root of the hierarchy
    wrm_elements.used++;
    wrm_elements.is_used[0] = true;
    wrm_Menu_Element *root = wrm_Pool_AS(wrm_elements, wrm_Menu_Element);

    root->parent = 0;
    root->visible = false;
    root->show_children = true;

    SDL_Window *w = wrm_render_getWindow();
    SDL_GetWindowSize(w, &wrm_window_width, &wrm_window_height);
    ndc_scale_h = 2.0f / wrm_window_height;
    ndc_scale_w = 2.0f / wrm_window_width;

    if(wrm_menu_settings.test) {
        wrm_menu_createTestElement();
    }

    return true;
}

void wrm_menu_update(void)
{

}

void wrm_menu_quit(void)
{
    for(u32 i = 0; i < wrm_fonts.len; i++) {
        free(wrm_fonts.data[i].glyph_set);
    }
    free(wrm_fonts.data);

    free(wrm_mesh_buf.positions);
    free(wrm_mesh_buf.uvs);
    free(wrm_mesh_buf.colors);
    free(wrm_mesh_buf.indices);

    FT_Done_FreeType(wrm_ft_library);
}

// font

wrm_Option_Handle wrm_menu_loadFont(const char *path)
{
    if(!realloc(wrm_fonts.data, sizeof(wrm_Font) * wrm_fonts.cap * 2)) {
        fprintf(stderr, "Failed to allocate space for new font\n");
        return OPTION_NONE(Handle);
    }
    
    wrm_Handle f_handle = wrm_fonts.len++;
    wrm_Font f = wrm_fonts.data[f_handle];

    f.glyph_set = calloc(WRM_FONT_GLYPHS_COUNT, sizeof(wrm_Glyph));

    // use freetype

    int error = FT_New_Face(wrm_ft_library, path, 0, &f.face);
    if(error) {
        fprintf(stderr, "ERROR: Menu: failed to load font face from path: '%s'\n", path);
        return OPTION_NONE(Handle);
    }

    FT_Set_Pixel_Sizes(f.face, 0, 48);
    FT_GlyphSlot g = f.face->glyph;
    int w = 0;
    int h = 0;

    for(u8 i = 32; i < 128; i++) {
        if(FT_Load_Char(f.face, i, FT_LOAD_RENDER)) {
            fprintf(stderr, "Loading character %c failed!\n", i);
            continue;
        }

        w += g->bitmap.width;
        h =  h > g->bitmap.rows ? h : g->bitmap.rows; 
    }

    // create a GL texture for the character atlas
    wrm_Texture_Data d = {
        .font = true,
        .height = h,
        .width = w,
        .pixels = NULL
    };

    wrm_Option_Handle result = wrm_render_createTexture(&d);

    if(!result.exists) {
        fprintf(stderr, "Error: Menu: failed to create texture for font loaded from: '%s'\n", path);
        return OPTION_NONE(Handle);
    }

    f.atlas = result.Handle_val;
    u32 x = 0;

    for(u32 i = 32; i < 128; i++) {
        if(FT_Load_Char(f.face, i, FT_LOAD_RENDER)) {
            continue;
        }

        f.glyph_set[i - 32] = (wrm_Glyph) {
            .advance_x = g->advance.x >> 6,
            .advance_y = g->advance.y >> 6,
            .bmp_w = g->bitmap.width,
            .bmp_h = g->bitmap.rows,
            .bmp_left = g->bitmap_left,
            .bmp_top = g->bitmap_top,
            .tex_offset = (float)x / w
        };
        
        d.width = g->bitmap.width;
        d.height = g->bitmap.rows;
        d.pixels = g->bitmap.buffer;

        wrm_render_updateTexture(f.atlas, &d, x, 0);

        x += g->bitmap.width;
    }

    FT_Done_Face(f.face);
    return (wrm_Option_Handle){ .exists = true, .Handle_val = f_handle };
}

// text box

wrm_Option_Handle wrm_menu_createTextBox(wrm_Text_Box *data, float x, float y, u32 width, u32 height, wrm_Handle parent)
{
    wrm_Option_Handle slot = wrm_Pool_getSlot(&wrm_elements);

    
    if(!slot.exists) {
        if(wrm_menu_settings.errors) { wrm_error("Menu: createTextBox()", "failed to find memory for text box"); }
        return slot;
    }

    if(parent && !wrm_menu_exists(parent, "createTextBox()", "element", "(parent value)")) {
        wrm_Pool_freeSlot(&wrm_elements, slot.Handle_val);
        return OPTION_NONE(Handle);
    }

    wrm_Menu_Element *tb = wrm_Pool_AS(wrm_elements, wrm_Menu_Element) + slot.Handle_val;

    *tb = (wrm_Menu_Element){
        .type = WRM_TEXT_BOX,
        .self.text_box = *data,
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .child_count = 0,
        .children = {0, 0, 0, 0},
        .visible = true,
        .show_children = false,
    };

    // create associated GUI data
    wrm_Option_Handle model = wrm_menu_createRenderData(tb);
    if(!model.exists) {
        if(wrm_menu_settings.errors) { wrm_error("Menu: createTextBox()", "failed to create rendering data for the text box"); }
        return model;
    }

    tb->model = model.Handle_val;
    
    // wrm_menu_updateTextBox(slot.Handle_val, data);

    if(parent) {
        wrm_menu_addChild(parent, slot.Handle_val); // need to do this after creating render data so models are updated
    }

    return slot;
}

void wrm_menu_updateTextBox(wrm_Handle textbox, wrm_Text_Box *t)
{
    return;
}

// pane

wrm_Option_Handle wrm_menu_createPane(wrm_Pane *p, u32 x, u32 y, u32 width, u32 height, wrm_Handle parent)
{
    return OPTION_NONE(Handle);
}

// general menu hierarchy

void wrm_menu_addChild(wrm_Handle parent, wrm_Handle child)
{
    char * caller = "addChild()";
    if(!(wrm_menu_exists(parent, caller, "", "(parent)") && wrm_menu_exists(child, caller, "", "(child)"))) {
        return;
    }

    wrm_Menu_Element *m = wrm_Pool_AS(wrm_elements, wrm_Menu_Element);

    if(m[parent].child_count == WRM_MODEL_CHILD_LIMIT) {
        fprintf(stderr, "ERROR: Render: cannot add another child to model [%u] (limit reached!)\n", parent); 
        return;
    }

    if(m[child].parent) {
        fprintf(stderr, "ERROR: Render: cannot add parent to model [%u] (already has parent!)\n", child); 
        return;
    }

    for(u8 i = 0; i < m[parent].child_count; i++) {
        if(m[parent].children[i] == child) {
            fprintf(stderr, "ERROR: Render: cannot add child [%u] to model [%u] (already added!)\n", child, parent); 
            return;
        }
    }

    if(parent) m[parent].children[m[parent].child_count++] = child;
    m[child].parent = parent;
}

void wrm_menu_removeChild(wrm_Handle parent, wrm_Handle child) 
{
    char * caller = "removeChildModel()";
    char * type = "element";
    if(!(wrm_menu_exists(parent, caller, type, "(parent)") && wrm_menu_exists(child, caller, type, "(child)"))) {
        return;
    }

    wrm_Menu_Element *m = wrm_Pool_AS(wrm_elements, wrm_Menu_Element);

    if(m[parent].child_count == 0) {
        fprintf(stderr, "ERROR: Menu: cannot remove child from model [%u] (has no children!)\n", parent); 
        return;
    }

    if(!m[child].parent) {
        fprintf(stderr, "ERROR: Menu: cannot remove parent from model [%u] (has no parent!)\n", child); 
        return;
    }

    if(m[child].parent != parent) {
        fprintf(stderr, "ERROR: Menu: cannot remove parent [%u] from model [%u] (has a different parent!)\n", parent, child); 
        return;
    }   

    
    
    // find the child
    u8 idx;
    for(idx = 0; idx < m[parent].child_count; idx++) {
        if(m[parent].children[idx] == child) break;
    }
    if(idx == WRM_MODEL_CHILD_LIMIT) {
        fprintf(stderr, "ERROR: Menu: cannot remove child [%u] from model [%u] (already removed!)\n", child, parent); 
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



/*
Internal helper definitions
*/

internal inline void wrm_menu_setQuadPositions(wrm_Quad *q, vec2 tl, float width, float height)
{
    if(!q) return;

    float l_x = tl[0];
    float t_y = tl[1];
    float r_x = l_x + width;
    float b_y = t_y - height;
    float z = 0.0f;
    
    glm_vec3_copy((float[]){l_x, t_y, z}, q->tl.pos);
    glm_vec3_copy((float[]){r_x, t_y, z}, q->tr.pos);
    glm_vec3_copy((float[]){l_x, b_y, z}, q->bl.pos);
    glm_vec3_copy((float[]){r_x, b_y, z}, q->br.pos);
}

internal inline void wrm_menu_setQuadColors(wrm_Quad *q, wrm_RGBAf tl, wrm_RGBAf tr, wrm_RGBAf bl, wrm_RGBAf br)
{
    if(!q) return;

    glm_vec4_copy((float[]){tl.r, tl.g, tl.b, tl.a}, q->tl.col);
    glm_vec4_copy((float[]){tr.r, tr.g, tr.b, tr.a}, q->tr.col);
    glm_vec4_copy((float[]){bl.r, bl.g, bl.b, bl.a}, q->bl.col);
    glm_vec4_copy((float[]){br.r, br.g, br.b, br.a}, q->br.col);

    q->has_col = true;
}

internal inline void wrm_menu_setQuadUVs(wrm_Quad *q, vec2 tl, float width, float height)
{
    if(!q) return;

    float l_x = tl[0];
    float t_y = tl[1];
    float r_x = l_x + width;
    float b_y = t_y + height;
    
    glm_vec2_copy((float[]){l_x, t_y}, q->tl.uv);
    glm_vec2_copy((float[]){r_x, t_y}, q->tr.uv);
    glm_vec2_copy((float[]){l_x, b_y}, q->bl.uv);
    glm_vec2_copy((float[]){r_x, b_y}, q->br.uv);

    q->has_uv = true;
}

internal inline void wrm_menu_resetMeshBuffer(wrm_Mesh_Buffer *m)
{
    if(!m) return;

    m->vtx_len = 0;
    m->idx_len = 0;
}

internal inline bool wrm_menu_addVertex(wrm_Mesh_Buffer *m, wrm_Vertex *v, bool colors, bool uvs)
{    
    u32 i = m->vtx_len;

    if(m->vtx_len == m->vtx_cap) {
        bool success = 
            realloc(m->positions, m->vtx_cap * 2 * 3 * sizeof(float)) && 
            realloc(m->colors, m->vtx_cap * 2 * 4 * sizeof(float)) && 
            realloc(m->uvs, m->vtx_cap * 2 * 2 * sizeof(float));

        if(!success) {
            wrm_error("Menu: addVertex():", "failed to allocate space for %u vertices in mesh buffer!", m->vtx_cap * 2);
            return false;
        }
        m->vtx_cap *= 2;
    }


    m->positions[3 * i] = v->pos[0];
    m->positions[3 * i + 1] = v->pos[1];
    m->positions[3 * i + 2] = v->pos[2];

    if(colors) {
        m->colors[4 * i] = v->col[0];
        m->colors[4 * i + 1] = v->col[1];
        m->colors[4 * i + 2] = v->col[2];
        m->colors[4 * i + 3] = v->col[3];
    }
    
    if(uvs) {
        m->uvs[2 * i] = v->uv[0];
        m->uvs[2 * i + 1] = v->uv[1];
    }
    
    m->vtx_len++;
    return true;
}

internal inline bool wrm_menu_addIndices(wrm_Mesh_Buffer *m, u32* indices, u8 count)
{
    if(m->idx_len + count >= m->idx_cap) {
        if(!realloc(m->indices, m->idx_cap * 2 * sizeof(u32))) {
            wrm_error("Menu: addIndices():", "failed to allocate space for %u indices in mesh buffer!", m->vtx_cap * 2);
            return false;
        }
        m->idx_cap *= 2;
    }
    for(u8 i = 0; i < count; i++) {
        m->indices[m->idx_len++] = indices[i];
    }
    return true;
}

internal inline bool wrm_menu_addQuad(wrm_Mesh_Buffer *m, wrm_Quad *q)
{
    if(!q || !m) return false;

    if(!(q->has_col || q->has_uv)) {
        wrm_error("Menu: addQuad()", "quad must have uvs or colors");
        return false;
    }


    u32 idx = m->vtx_len;
    // add four vertices
    bool success = 
        wrm_menu_addVertex(&wrm_mesh_buf, &q->tl, q->has_col, q->has_uv) &&
        wrm_menu_addVertex(&wrm_mesh_buf, &q->tr, q->has_col, q->has_uv) &&
        wrm_menu_addVertex(&wrm_mesh_buf, &q->bl, q->has_col, q->has_uv) &&
        wrm_menu_addVertex(&wrm_mesh_buf, &q->br, q->has_col, q->has_uv);

    if(!success) {
        wrm_error("Menu: addQuad()", "failed to add all vertices");
        return false;
    } 

    // add six indices (CW)
    u32 indices[] = {idx, idx + 1, idx + 3, idx, idx + 3, idx + 2};

    if(!wrm_menu_addIndices(&wrm_mesh_buf, indices, 6)) {
        wrm_error("Menu: addQuad()", "failed to add indices");
        return false;
    }
    return true;
}

internal wrm_Option_Handle wrm_menu_createRenderData(wrm_Menu_Element *e)
{
    if(!e) return OPTION_NONE(Handle);

    // reset mesh buffer
    wrm_menu_resetMeshBuffer(&wrm_mesh_buf);


    // convert pixel width and height to NDC coords for model
    float width = (float)e->width * ndc_scale_w;
    float height = (float)e->height * ndc_scale_h;
    // initialize with 0.0, 0.0 as the top left corner of the quad
    vec2 tl = { 0.0f, 0.0f };

    // start with just a simple error box
    wrm_Quad q;
    wrm_menu_setQuadPositions(&q, tl, width, height);
    vec2 tl_uv = {0.0f, 0.0f};
    float uv_w = 1.0f;
    float uv_h = 1.0f;
    wrm_menu_setQuadUVs(&q, tl_uv, uv_w, uv_h);

    wrm_menu_addQuad(&wrm_mesh_buf, &q);

    wrm_Mesh_Data mesh_data = {
        .cw = true,
        .dynamic = true,
        .mode = WRM_MESH_TRIANGLE,
        .positions = wrm_mesh_buf.positions,
        .colors = NULL,
        .uvs = wrm_mesh_buf.uvs,
        .indices = wrm_mesh_buf.indices,
        .idx_cnt = wrm_mesh_buf.idx_len,
        .vtx_cnt = wrm_mesh_buf.vtx_len
    };

    wrm_Option_Handle result = wrm_render_createMesh(&mesh_data);
    if(!result.exists) {
        return result;
    }
    printf("Menu: mesh created! [%u]\n", result.Handle_val);

    wrm_Model model_data = {
        .pos = {e->x, e->y, 0.0f},
        .rot = {0.0f, 0.0f, 0.0f},
        .scale = {1.0f, 1.0f, 1.0f},
        .mesh = result.Handle_val,
        .texture = 0,
        .shader = wrm_shader_defaults.texture,
        .is_visible = true,
        .is_ui = true
    };

    result = wrm_render_createModel(&model_data, true);
    return result;
}

internal wrm_Option_Handle wrm_menu_updateTextBoxMesh(wrm_Menu_Element *e)
{
    return OPTION_NONE(Handle);
}

internal bool wrm_menu_exists(wrm_Handle h, const char *caller, const char *type, const char *reason)
{
    if(wrm_elements.is_used[h]) { return true; }

    fprintf(stderr, "ERROR: Menu: %s: %s [%u] does not exist %s\n", caller, type, h, reason);
    return false;
}

internal void wrm_menu_createTestElement(void)
{

    wrm_Text_Box tb = {
        .font = 0,
        .text = "Hello menu!",
        .text_len = strlen("Hello menu!"),
    };

    wrm_Option_Handle result = wrm_menu_createTextBox(&tb, -0.5f, 0.5f, 50, 70, 0);
    if(!result.exists) {
        if(wrm_menu_settings.errors) { wrm_error("Menu: init()", "failed to create test element!"); }
        return;
    }
    if(wrm_menu_settings.verbose) { printf("Menu: successfully created test element: [%u]\n", result.Handle_val); }
}

# endif // end implementation

#endif // end header