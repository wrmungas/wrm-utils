#include "gui.h"


wrm_Option_Handle wrm_gui_createPane(wrm_gui_Properties properties, wrm_RGBA color)
{
    wrm_Option_Handle result = wrm_Pool_getSlot(&wrm_gui_elements);

    if(!result.exists) return result;

    wrm_Pane *pane = (wrm_Pane*)wrm_Pool_AT(wrm_gui_elements, wrm_gui_Element, result.val);
    
    pane->properties = properties;

    pane->properties.type = WRM_GUI_PANE; // force the correct type regardless of passed-in properties
    pane->color = color;

    return result;
}

void wrm_gui_drawPane(wrm_Pane *p)
{
    // get the top-left position and dimensions in NDC
    vec2 tl;
    vec2 dim;
    wrm_gui_NDC_fromAlignment(p->properties.alignment, tl, dim);

    // set up quad
    wrm_gui_setQuadCorners(tl, dim);

    wrm_RGBAf cf = wrm_RGBAf_fromRGBA(p->color);

    wrm_gui_setQuadCol(WRM_QUAD_TL, cf);
    wrm_gui_setQuadCol(WRM_QUAD_TR, cf);
    wrm_gui_setQuadCol(WRM_QUAD_BL, cf);
    wrm_gui_setQuadCol(WRM_QUAD_BR, cf);

    wrm_gui_updateQuad();

    // render
    wrm_render_setGLShader(wrm_gui_pane_shader);
    glBindVertexArray(the_quad.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}