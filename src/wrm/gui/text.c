#include "gui.h"

/*
Sets the quad data and draws a single character to the screen from the given 'current point' and updates it
*/
static void wrm_gui_drawCharacter(char c, wrm_Font *f, ivec2 current_point);

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

    e->text.properties.type = WRM_GUI_TEXT;

    return result;
}

void wrm_gui_drawText(wrm_Text *t)
{
    if(!t) return;

    
    // set GL state
    wrm_render_setGLShader(wrm_gui_text_shader);

    wrm_Shader *s = wrm_Pool_at(&wrm_shaders, wrm_gui_text_shader);
    GLint color_loc = glGetUniformLocation(s->program, "text_col");
    if(color_loc != -1) {
        wrm_RGBAf color = wrm_RGBAf_fromRGBA(t->text_color);
        glUniform3fv(color_loc, 1, (float[]){color.r, color.g, color.b});
    }

    wrm_Font *f = wrm_Stack_at(&wrm_fonts, t->font);
    wrm_render_setGLTexture(f->atlas);

    ivec2 start_point;
    wrm_gui_getTopLeft(t->properties.alignment, &start_point[WRM_X], &start_point[WRM_Y]);
    start_point[WRM_Y] -= f->y_max;

    ivec2 current_point;
    glm_ivec2_copy(start_point, current_point);

    u32 i = 0;
    while(t->src_text[i]) {
        //TODO: add checks that the current point plus the advance isn't longer than the text box width
        if(t->src_text[i] == '\n') {
            current_point[WRM_X] = start_point[WRM_X];
            current_point[WRM_Y] += f->y_max + f->y_min + t->line_spacing; 
        }
        else {
            wrm_gui_drawCharacter(t->src_text[i], f, current_point);
        }
        i++;
    }
}

/*
File internal
*/


static void wrm_gui_drawCharacter(char c, wrm_Font *f, ivec2 current_point)
{
    wrm_Glyph *g = &(f->glyphs[c - 32]);

    wrm_gui_Alignment a = {
        .height = g->h_px,
        .width = g->w_px,
        .x = current_point[WRM_X] + g->bearing_x,
        .x_is = WRM_LEFT,
        .x_from = WRM_LEFT,
        .y = current_point[WRM_Y] - g->bearing_y,
        .y_is = WRM_TOP,
        .y_from = WRM_TOP
    };

    vec2 tl;
    vec2 dim;
    wrm_gui_NDC_fromAlignment(a, tl, dim);

    wrm_gui_setQuadCorners(tl, dim);

    wrm_gui_setQuadUV(WRM_QUAD_TL, (vec2){g->tl_u, 0.0f});
    wrm_gui_setQuadUV(WRM_QUAD_TR, (vec2){g->tl_u + g->w, 0.0f});
    wrm_gui_setQuadUV(WRM_QUAD_BL, (vec2){g->tl_u, g->h});
    wrm_gui_setQuadUV(WRM_QUAD_BR, (vec2){g->tl_u + g->w, g->h});

    wrm_gui_updateQuad();

    glBindVertexArray(the_quad.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    current_point[WRM_X] += g->advance;    
}