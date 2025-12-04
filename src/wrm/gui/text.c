#include "gui.h"

/* draws from a position in a string up until the next newline character, or NULL; returns the number of chars drawn */
static u32 wrm_gui_drawTextLine(const char *s, wrm_Font *f, ivec2 start_point);
/* Sets the quad data and draws a single character to the screen */
static void wrm_gui_drawCharacter(char c, wrm_Font *f, ivec2 point);

/* 
Module internal
*/

wrm_Option_Handle wrm_gui_createText(wrm_gui_Properties properties, wrm_Handle font, wrm_RGBA text_color, const char *src, u32 spacing)
{
    wrm_Option_Handle result = wrm_Pool_getSlot(&wrm_gui_elements);
    if(!result.exists) return result;

    wrm_gui_Element *e = wrm_Pool_at(&wrm_gui_elements, result.val);
    if(!e) return OPTION_NONE(Handle);

    e->text = (wrm_Text) {
        .properties = properties,
        .text_color = text_color,
        .src_text = src,
        .line_spacing = spacing
    };

    return result;
}

void wrm_gui_drawText(wrm_Text *t)
{
    if(!t) return;

    u32 i = 0;
    wrm_Font *f = wrm_Stack_at(&wrm_fonts, t->font);

    ivec2 start = { 
        t->properties.alignment.x,
        t->properties.alignment.y
    };

    wrm_Shader *s = wrm_Pool_at(&wrm_shaders, wrm_gui_text_shader);

    // set GL state
    glUseProgram(s->program);
    GLint color_loc = glGetUniformLocation(s->program, "text_col");
    
    if(color_loc != -1) {
        wrm_RGBAf color = wrm_RGBAf_fromRGBA(t->text_color);
        glUniform3fv(color_loc, 1, (float[]){color.r, color.g, color.b});
    }

    wrm_Texture *r_tex = wrm_Pool_at(&wrm_textures, f->atlas);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r_tex->gl_tex);

    while(t->src_text[i]) {
        i += wrm_gui_drawTextLine(t->src_text + i, f, start);
        start[WRM_Y] += f->atlas_height + t->line_spacing;
    }
}

/*
File internal
*/

static u32 wrm_gui_drawTextLine(const char *s, wrm_Font *f, ivec2 start_point)
{
    ivec2 point;
    point[WRM_X] = start_point[WRM_X];
    point[WRM_Y] = start_point[WRM_Y];

    u32 i = 0;
    while(s[i] && s[i] != '\n') {
        wrm_gui_drawCharacter(s[i], f, point);
    }

    return i + 1;
}

static void wrm_gui_drawCharacter(char c, wrm_Font *f, ivec2 point)
{
    // convert pixel coordinates to screen space
    
}