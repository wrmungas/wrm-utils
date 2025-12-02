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

    wrm_Font *f = wrm_Stack_at(&wrm_fonts, f_handle.val);

    if(!f) { return OPTION_NONE(Handle); }

    f->glyph_set = calloc(WRM_FONT_GLYPHS_COUNT, sizeof(wrm_Glyph));

    // use freetype

    int error = FT_New_Face(wrm_ft_library, path, 0, &(f->face));
    if(error) {
        fprintf(stderr, "ERROR: Menu: failed to load font face from path: '%s'\n", path);
        return OPTION_NONE(Handle);
    }

    FT_Set_Pixel_Sizes(f->face, 0, 48);
    
    int w = 0;
    int h = 0;
    FT_GlyphSlot g = f->face->glyph;

    // iterate once over the characters to get the maximum width and height
    for(u8 c = 32; c < 128; c++) {
        if(FT_Load_Char(f->face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "Loading character %c failed!\n", c);
            continue;
        }
        
        w += g->bitmap.width;
        h =  h > g->bitmap.rows ? h : g->bitmap.rows; 
    }

    // allocate a pixel buffer to copy the bitmaps into
    
    u8 *pixels = calloc(w * h, 1);
    u32 x_offset = 0; 

    for(u32 c = 32; c < 128; c++) {
        if(FT_Load_Char(f->face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "Loading character %c failed!\n", c);
            continue;
        }

        f->glyph_set[c - 32] = (wrm_Glyph) {
            .advance_x = g->advance.x >> 6, // advance is given in units of 1/64th pixel, 64 = 2^6
            .advance_y = g->advance.y >> 6,
            .bmp_w = g->bitmap.width,
            .bmp_h = g->bitmap.rows,
            .bmp_left = g->bitmap_left,
            .bmp_top = g->bitmap_top,
            .tex_offset = (float)x_offset / w
        };
        
        // copy pixel data over
        for(u32 j = 0; j < g->bitmap.rows; j++) {
            for(u32 i = 0; i < g->bitmap.width; i++) {
                pixels[j * w + i + x_offset] = g->bitmap.buffer[j * g->bitmap.width + i];
            }
        }
        x_offset += g->bitmap.width;
    }

    // create a GL texture for the character atlas
    wrm_Texture_Data d = {
        .channels = 1,
        .height = h,
        .width = w,
        .pixels = pixels
    };
    wrm_Option_Handle result = wrm_render_createTexture(&d);
    if(!result.exists) {
        fprintf(stderr, "Error: Menu: failed to create texture for font loaded from: '%s'\n", path);
        return OPTION_NONE(Handle);
    }

    f->atlas = result.val;

    FT_Done_Face(f->face);
    return f_handle;
}