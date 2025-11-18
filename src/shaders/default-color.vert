// file: default-color.vert
#version 330 core

layout (location = 0) in vec3 v_pos; // positions are location 0
layout (location = 1) in vec4 v_col; // colors are location 1

uniform mat4 mvp;

out vec4 col; // specify a color output to the fragment shader

void main()
{
    gl_Position = mvp * vec4(v_pos, 1.0);
    col = v_col;
}