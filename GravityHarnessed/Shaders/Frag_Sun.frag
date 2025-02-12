#version 430

in vec3 vs_out_pos;
in vec2 vs_out_tex;

out vec4 fs_out_col;

uniform sampler2D colorTexImage;

void main()
{
	fs_out_col = texture(colorTexImage, vs_out_tex);
}