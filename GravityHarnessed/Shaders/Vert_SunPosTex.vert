#version 430

// VBO-ból érkező változók
layout( location = 0 ) in vec3 vs_in_pos;
//layout( location = 1 ) in vec3 vs_in_norm;
layout( location = 1 ) in vec2 vs_in_tex;

// a pipeline-ban tovább adandó értékek
out vec3 vs_out_pos;
out vec2 vs_out_tex;

// shader külső paraméterei - most a három transzformációs mátrixot külön-külön vesszük át
uniform mat4 world;
uniform mat4 viewProj;

uniform sampler2D colorTexImage;
uniform sampler2D heightTexImage;

uniform float noiseTranslate;

float r = 1.0;

vec3 GetPos(float u, float udif, float v, float vdif, vec2 temp_tex);

// SUN
void main()
{
	float u = vs_in_pos.x * 3.1415 * 2.0001 ;
	float v = vs_in_pos.y * 3.1415;

	vs_out_pos = vec3( 
			r * sin(v) * cos(u),
			r * cos(v),
			r * sin(v) * sin(u));

	vs_out_tex = vs_in_tex;
	vs_out_tex.y = 1 - vs_out_tex.y;
	vs_out_tex.x = 1 - vs_out_tex.x;

	if(vs_out_tex.x == 1.0)
	{
		vs_out_pos = vs_out_pos * (1.0 + length(texture(heightTexImage, vec2(0.0 + noiseTranslate, vs_out_tex.y))) * 0.1);
	}
	else
	{
		vs_out_pos = vs_out_pos * (1.0 + length(texture(heightTexImage, vec2(vs_out_tex.x * sin(vs_out_tex.y * 3.1415) + noiseTranslate, vs_out_tex.y))) * 0.1);
	}

	gl_Position = viewProj * world * vec4( vs_out_pos, 1 );
	vs_out_pos  = (world   * vec4(vs_out_pos,  1)).xyz;

}