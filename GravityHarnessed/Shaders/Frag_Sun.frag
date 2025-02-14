#version 430

in vec3 vs_out_pos;
in vec2 vs_out_tex;

layout (location = 0) out vec4 fs_out_col;
layout (location = 1) out vec4 fs_out_justSun;

uniform sampler2D colorTexImage;

void main()
{
	fs_out_col = texture(colorTexImage, vs_out_tex);
	fs_out_justSun = fs_out_col;
}