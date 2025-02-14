#version 430

// Per fragment variables coming from the pipeline
in vec2 vs_out_tex;

// Outgoing values - fragment color
out vec4 fs_out_col;

// External parameters of the shader
uniform sampler2D fullScreen;
uniform sampler2D justSun;

uniform vec2 res = vec2(800,600);

void main()
{
 	fs_out_col = texture(fullScreen, vs_out_tex);
}