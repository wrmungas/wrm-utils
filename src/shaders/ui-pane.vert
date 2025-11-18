// file: ui-pane.vert
#version 330 core 

layout (location = 0) in vec2 v_pos; 
layout (location = 1) in vec4 v_col; 
layout (location = 2) in vec2 v_uv; // ignored but needed for static vao

out vec4 col; // specify a uv for the fragment shader

void main()
{
    gl_Position = vec4(v_pos, 1.0, 1.0);
    col = v_col;
}