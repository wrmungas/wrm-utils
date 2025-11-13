#include "render.h"

// mesh constants/globals

const static u8 floats_per_pos = 3;
const static u8 floats_per_col = 4;
const static u8 floats_per_uv = 2;

// default meshes
const wrm_Mesh_Data default_meshes_colored_triangle = {
    .positions = (float[]) {
        -0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
    },
    .colors = (float[]){
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
    },
    .uvs = NULL,
    .indices = (u32[]) {
        0, 1, 2
    },
    .cw = true,
    .idx_cnt = 3,
    .vtx_cnt = 3,
    .mode = GL_TRIANGLES,
};

const wrm_Mesh_Data default_meshes_colored_cube = {
    .positions = (float[]) {
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
    },
    .colors = (float[]) {
        0.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
    },
    .uvs = NULL,
    .indices = (u32[]) {
        0, 1, 3, 0, 3, 2,
        1, 5, 7, 1, 7, 3,
        5, 4, 6, 5, 6, 7,
        4, 0, 2, 4, 2, 6,
        4, 5, 1, 4, 1, 0,
        2, 3, 7, 2, 7, 6,
    },
    .cw = true,
    .idx_cnt = 36,
    .vtx_cnt = 8,
    .mode = GL_TRIANGLES,
};

const wrm_Mesh_Data default_meshes_textured_cube = {
    .positions = (float[]) {
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f, -0.5f,

         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
    },
    .colors = (float[]) {
        0.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        
        0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,

        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,

        1.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,

        0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
    },
    .uvs = (float[]) {
        0.0f, 0.0f, 1.0f, 0.0f, 
        0.0f, 1.0f, 1.0f, 1.0f,

        0.0f, 0.0f, 1.0f, 0.0f, 
        0.0f, 1.0f, 1.0f, 1.0f,

        0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 1.0f, 0.0f, 
        0.0f, 1.0f, 1.0f, 1.0f,

        0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 
    },
    .indices = (u32[]) {
        0, 1, 3, 0, 3, 2, 
        4, 5, 7, 4, 7, 6, 
        8, 9, 11, 8, 11, 10,
        12, 13, 15, 12, 15, 14,
        16, 17, 19, 16, 19, 18,
        20, 21, 23, 20, 23, 22,
    },
    .cw = true,
    .idx_cnt = 36,
    .vtx_cnt = 24,
    .mode = GL_TRIANGLES,
};

// user-visible

wrm_Option_Handle wrm_render_createMesh(const wrm_Mesh_Data *data)
{
    if(!data) { return OPTION_NONE(Handle); }
    wrm_Option_Handle result = wrm_Pool_getSlot(&wrm_meshes);
    GLuint vtx_attrib;
    GLuint gl_draw = data->dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    
    GLuint pos_vbo = 0;
    glGenBuffers(1, &pos_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
    glBufferData(GL_ARRAY_BUFFER, data->vtx_cnt * floats_per_pos * sizeof(float), data->positions, gl_draw);

    vtx_attrib = WRM_SHADER_ATTRIB_POS_LOC;
    glVertexAttribPointer(vtx_attrib, floats_per_pos, GL_FLOAT, GL_FALSE, floats_per_pos * sizeof(float), (void*)0);
    glEnableVertexAttribArray(vtx_attrib);

    GLuint col_vbo = 0;
    if(data->colors) {
        glGenBuffers(1, &col_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, col_vbo);
        glBufferData(GL_ARRAY_BUFFER, data->vtx_cnt * floats_per_col * sizeof(float), data->colors, gl_draw);

        vtx_attrib = WRM_SHADER_ATTRIB_COL_LOC;
        glVertexAttribPointer(vtx_attrib, floats_per_col, GL_FLOAT, GL_FALSE, floats_per_col * sizeof(float), (void*)0);
        glEnableVertexAttribArray(vtx_attrib);
    }

    GLuint uv_vbo = 0;
    if(data->uvs) {
        glGenBuffers(1, &uv_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
        glBufferData(GL_ARRAY_BUFFER, data->vtx_cnt * floats_per_uv * sizeof(float), data->uvs, gl_draw);

        vtx_attrib = WRM_SHADER_ATTRIB_UV_LOC;
        glVertexAttribPointer(vtx_attrib, floats_per_uv, GL_FLOAT, GL_FALSE, floats_per_uv * sizeof(float), (void*)0);
        glEnableVertexAttribArray(vtx_attrib);
    }

    
    GLuint ebo = 0;
    if(data->indices) {
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        // handle indices for different drawing modes

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data->idx_cnt * sizeof(u32), data->indices, gl_draw);
    }
    

    wrm_Mesh m = {
        .vao = vao,
        .pos_vbo = pos_vbo,
        .col_vbo = col_vbo,
        .uv_vbo = uv_vbo,
        .ebo = ebo,
        .count = data->idx_cnt,
        .mode = data->mode, 
        .cw = data->cw,
    };

    ((wrm_Mesh*)wrm_meshes.data)[result.val] = m;
    return result;
}

wrm_Option_Handle wrm_render_cloneMesh(wrm_Handle mesh)
{
    // TODO: look up how to duplicate data that has been sent to the GPU
    return OPTION_NONE(Handle);
}

bool wrm_render_updateMesh(wrm_Handle mesh, const wrm_Mesh_Data *data)
{
    return true;
}

void wrm_render_printMeshData(wrm_Handle mesh)
{
    if(!wrm_render_exists(mesh, WRM_RENDER_RESOURCE_MESH, "printMeshData()", "")) return;

    wrm_Mesh *m = wrm_Pool_dataAs(wrm_meshes, wrm_Mesh);
    u32 i = mesh;
    printf(
        "[%u]: { vao: %u, pos_vbo: %u, col_vbo: %u, uv_vbo: %u, ebo: %u, count: %zu, cw: %s, mode: %u }\n", 
        i,
        m[i].vao,
        m[i].pos_vbo,
        m[i].col_vbo, 
        m[i].uv_vbo,
        m[i].ebo,
        m[i].count,
        m[i].cw ? "true" : "false",
        m[i].mode
    );
}

// module internal