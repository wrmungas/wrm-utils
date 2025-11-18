#include "gui.h"

wrm_Quad the_quad; // I love how ominous this sounds

void wrm_gui_initQuad(void)
{
    glGenVertexArrays(1, &the_quad.vao);
    glBindVertexArray(the_quad.vao);

    // positions
    wrm_render_createVBO(&the_quad.pos_vbo, WRM_SHADER_ATTRIB_POS_LOC, 4, 2, the_quad.positions, GL_DYNAMIC_DRAW);

    // colors
    wrm_render_createVBO(&the_quad.col_vbo, WRM_SHADER_ATTRIB_COL_LOC, 4, 4, the_quad.colors, GL_DYNAMIC_DRAW);

    // uvs
    wrm_render_createVBO(&the_quad.uv_vbo, WRM_SHADER_ATTRIB_UV_LOC, 4, 2, the_quad.uvs, GL_DYNAMIC_DRAW);

    // indices are a little different than the rest, they will not be updated
    GLuint indices[6] = {0, 1, 2, 1, 3, 2};
    glGenBuffers(1, &the_quad.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, the_quad.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void wrm_gui_updateQuad(void)
{
    glBindVertexArray(the_quad.vao);

    glBindBuffer(GL_ARRAY_BUFFER, the_quad.pos_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(the_quad.positions), the_quad.positions);

    glBindBuffer(GL_ARRAY_BUFFER, the_quad.col_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(the_quad.colors), the_quad.colors);

    glBindBuffer(GL_ARRAY_BUFFER, the_quad.uv_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(the_quad.uvs), the_quad.uvs);
}

void wrm_gui_deleteQuad(void)
{
    glDeleteBuffers(4, (GLuint[]){the_quad.pos_vbo, the_quad.col_vbo, the_quad.uv_vbo, the_quad.ebo});
    glDeleteVertexArrays(1, &the_quad.vao);
}

// force the compiler to emit symbols for these

void wrm_gui_setQuadCorners(const vec2 tl, const vec2 dim);

void wrm_gui_setQuadPos(int vtx, const vec2 pos);

void wrm_gui_setQuadCol(int vtx, const wrm_RGBAf col);

void wrm_gui_setQuadUV(int vtx, const vec2 uv);

void wrm_gui_setQuadVert(int vtx, const vec2 pos, const wrm_RGBAf col, const vec2 uv);