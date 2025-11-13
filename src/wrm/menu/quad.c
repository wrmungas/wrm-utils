#include "menu.h"

// module internal

void wrm_menu_setQuadPositions(wrm_Quad *q, vec2 tl, float width, float height)
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

void wrm_menu_setQuadColors(wrm_Quad *q, wrm_RGBAf tl, wrm_RGBAf tr, wrm_RGBAf bl, wrm_RGBAf br)
{
    if(!q) return;

    glm_vec4_copy((float[]){tl.r, tl.g, tl.b, tl.a}, q->tl.col);
    glm_vec4_copy((float[]){tr.r, tr.g, tr.b, tr.a}, q->tr.col);
    glm_vec4_copy((float[]){bl.r, bl.g, bl.b, bl.a}, q->bl.col);
    glm_vec4_copy((float[]){br.r, br.g, br.b, br.a}, q->br.col);

    q->has_col = true;
}

void wrm_menu_setQuadUVs(wrm_Quad *q, vec2 tl, float width, float height)
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