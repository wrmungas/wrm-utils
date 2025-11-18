// file: default-texture.frag
#version 330 core

in vec2 uv;

uniform sampler2D tex;

out vec4 f_col;

void main()
{
    f_col = texture(tex, uv);
}