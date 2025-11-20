#include "render.h"

// mesh constants/globals

const static u8 floats_per_col = 4;
const static u8 floats_per_uv = 2;

// default meshes

// equilateral triangle centered at origin facing +x; colors: top = red, lower-right = green, lower-left = blue
const wrm_Mesh_Data default_meshes_colored_triangle = {
    .format = {
        .col = true,
        .per_pos = 3,
        .tex = false,
    },
    .positions = (float[]) {
        0.0f, 0.577530f, 0.0f,
        0.0f,-0.288675f,-0.5f,
        0.0f,-0.288675f, 0.5f,
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

// unit cube centered at zero, textured
const wrm_Mesh_Data default_meshes_colored_cube = {
    .format = {
        .col = true,
        .per_pos = 3,
        .tex = false,
    },
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
    .format = {
        .col = true,
        .per_pos = 3,
        .tex = true,
    },
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
    wrm_Mesh *mesh = wrm_Pool_at(&wrm_meshes, result.val);
    *mesh = (wrm_Mesh){
        .format = data->format,
        .vao = 0,
        .pos_vbo = 0,
        .col_vbo = 0,
        .uv_vbo = 0,
        .ebo = 0,
        .transparent = data->transparent,
        .cw = data->cw,
        .mode = data->mode,
        .count = data->idx_cnt,
    };

    GLenum gl_draw = data->dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    // always create position vbo
    if(!data->positions) { 
        wrm_error("Render", "createMesh()", "failed to create mesh - needs position data");
        wrm_render_deleteMesh(result.val);
        return OPTION_NONE(Handle); 
    }
    wrm_render_createVBO(
        &mesh->pos_vbo, WRM_SHADER_ATTRIB_POS_LOC, data->vtx_cnt, 
        mesh->format.per_pos, data->positions, gl_draw
    );
    
    
    if(mesh->format.col) {
        if(!data->colors) { 
            wrm_error("Render", "createMesh()", "failed to create mesh - needs color data");
            wrm_render_deleteMesh(result.val);
            return OPTION_NONE(Handle);
        }
        
        wrm_render_createVBO(
            &mesh->col_vbo, WRM_SHADER_ATTRIB_COL_LOC, data->vtx_cnt, 
            floats_per_col, data->colors, gl_draw
        );
    }
    
    if(mesh->format.tex) {
        if(!data->colors) { 
            wrm_error("Render", "createMesh()", "failed to create mesh - needs texture data");
            wrm_render_deleteMesh(result.val);
            return OPTION_NONE(Handle);
        }
        wrm_render_createVBO(
            &mesh->uv_vbo, WRM_SHADER_ATTRIB_UV_LOC, data->vtx_cnt, 
            floats_per_uv, data->uvs, gl_draw
        );
    }
    
    if(data->indices) {
        glGenBuffers(1, &mesh->ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
        // handle indices for different drawing modes ?
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data->idx_cnt * sizeof(u32), data->indices, gl_draw);
    }

    return result;
}

void wrm_render_createVBO(GLuint *vbo, u32 attr_loc, size_t num_entries, size_t values_per_entry, const void *data, GLenum usage)
{
    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, num_entries * values_per_entry * sizeof(float), data, usage);

    glVertexAttribPointer(attr_loc, values_per_entry, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(attr_loc);
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

void wrm_render_debugMesh(wrm_Handle mesh)
{
    if(!wrm_render_exists(mesh, WRM_RENDER_RESOURCE_MESH, "printMeshData()", "")) return;

    wrm_Mesh *m = wrm_Pool_at(&wrm_meshes, mesh);
    printf(
        "[%u]:\n "
        "{ format: { tex: %s, col: %s, per_pos: %u }, "
        "vao: %u, pos_vbo: %u, col_vbo: %u, uv_vbo: %u, "
        "ebo: %u, count: %zu, cw: %s, mode: %u }\n", 
        mesh,
        m->format.tex ? "true" : "false", 
        m->format.col ? "true" : "false",
        m->format.per_pos,
        m->vao,
        m->pos_vbo,
        m->col_vbo, 
        m->uv_vbo,
        m->ebo,
        m->count,
        m->cw ? "true" : "false",
        m->mode
    );
}

void wrm_render_deleteMesh(wrm_Handle mesh)
{
    wrm_Mesh_delete(wrm_Pool_at(&wrm_meshes, mesh));
    wrm_Pool_freeSlot(&wrm_meshes, mesh);
}

// module internal

void wrm_Mesh_delete(void *mesh)
{
    if(!mesh) return;
    wrm_Mesh *m = mesh;

    // silently ignores any of these that are 0
    glDeleteBuffers(4, (GLuint[]){ m->pos_vbo, m->col_vbo, m->uv_vbo, m->ebo});
    glDeleteVertexArrays(1, &m->vao);
}