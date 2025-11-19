#include "gui.h"

wrm_Option_Handle wrm_gui_createImage(wrm_gui_Properties properties, wrm_Handle texture)
{
    wrm_Option_Handle result = wrm_Pool_getSlot(&wrm_gui_elements);

    if(!result.exists) return result;

    wrm_gui_Element *e = wrm_Pool_at(&wrm_gui_elements, result.val);
    if(!e) { return OPTION_NONE(Handle); }

    wrm_Image *image = &e->image;
    
    image->properties = properties;

    image->properties.type = WRM_GUI_IMAGE; // force the correct type regardless of passed-in properties
    image->image_texture = texture;

    return result;
}

void wrm_gui_drawImage(wrm_Image *i)
{
    // get the top-left position and dimensions in NDC
    vec2 tl;
    vec2 dim;
    wrm_gui_NDC_fromAlignment(i->properties.alignment, tl, dim);

    // set up quad
    wrm_gui_setQuadCorners(tl, dim);

    wrm_gui_setQuadUV(WRM_QUAD_TL, (vec2){0.0f, 0.0f});
    wrm_gui_setQuadUV(WRM_QUAD_TR, (vec2){1.0f, 0.0f});
    wrm_gui_setQuadUV(WRM_QUAD_BL, (vec2){0.0f, 1.0f});
    wrm_gui_setQuadUV(WRM_QUAD_BR, (vec2){1.0f, 1.0f});

    wrm_gui_updateQuad();

    // render./b
    wrm_render_setGLShader(wrm_gui_image_shader);
    wrm_render_setGLTexture(i->image_texture);
    glBindVertexArray(the_quad.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}