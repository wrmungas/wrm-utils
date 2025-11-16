#include "gui.h"

/*
Constants
*/

const u8 WRM_FONT_GLYPHS_COUNT = 128 - 32;


// font operations

wrm_Option_Handle wrm_gui_loadFont(const char *path)
{   
    wrm_Option_Handle f_handle = wrm_Stack_push(&wrm_fonts);

    if(!f_handle.exists) { 
        wrm_error("Menu", "loadFont()", "could not get space for a new font (stack error)");
        return f_handle; 
    }

    wrm_Font *f = wrm_Stack_dataAs(wrm_fonts, wrm_Font) + f_handle.val;

    f->glyph_set = calloc(WRM_FONT_GLYPHS_COUNT, sizeof(wrm_Glyph));

    // use freetype

    int error = FT_New_Face(wrm_ft_library, path, 0, &(f->face));
    if(error) {
        fprintf(stderr, "ERROR: Menu: failed to load font face from path: '%s'\n", path);
        return OPTION_NONE(Handle);
    }

    FT_Set_Pixel_Sizes(f->face, 0, 48);
    FT_GlyphSlot g = f->face->glyph;
    int w = 0;
    int h = 0;

    for(u8 i = 32; i < 128; i++) {
        if(FT_Load_Char(f->face, i, FT_LOAD_RENDER)) {
            fprintf(stderr, "Loading character %c failed!\n", i);
            continue;
        }

        w += g->bitmap.width;
        h =  h > g->bitmap.rows ? h : g->bitmap.rows; 
    }

    // create a GL texture for the character atlas
    wrm_Texture_Data d = {
        .channels = 4,
        .height = h,
        .width = w,
        .pixels = NULL
    };

    wrm_Option_Handle result = wrm_render_createTexture(&d);

    if(!result.exists) {
        fprintf(stderr, "Error: Menu: failed to create texture for font loaded from: '%s'\n", path);
        return OPTION_NONE(Handle);
    }

    f->atlas = result.val;
    u32 x = 0;

    for(u32 i = 32; i < 128; i++) {
        if(FT_Load_Char(f->face, i, FT_LOAD_RENDER)) {
            continue;
        }

        f->glyph_set[i - 32] = (wrm_Glyph) {
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

        wrm_render_updateTexture(f->atlas, &d, x, 0);

        x += g->bitmap.width;
    }

    FT_Done_Face(f->face);
    return f_handle;
}