// file: ui-image.frag
#version 330 core

in vec2 uv;

uniform sampler2D tex;

out vec4 f_col;

void main()
{    
    vec4 sampled = texture(tex, uv);
    f_col = sampled;
}  