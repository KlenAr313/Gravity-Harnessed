#version 430

// VBO-ból érkező változók
layout( location = 0 ) in vec3 vs_in_pos;
//layout( location = 1 ) in vec3 vs_in_norm;
layout( location = 1 ) in vec2 vs_in_tex;

// a pipeline-ban tovább adandó értékek
out vec3 vs_out_pos;
out vec3 vs_out_norm;
out vec2 vs_out_tex;

// shader külső paraméterei - most a három transzformációs mátrixot külön-külön vesszük át
uniform mat4 world;
uniform mat4 worldIT;
uniform mat4 viewProj;

uniform sampler2D colorTexImage;
uniform sampler2D heightTexImage;

float r = 1.0;

vec3 GetPos(float u, float udif, float v, float vdif, vec2 temp_tex);

// PLANET
void main()
{
	float u = vs_in_pos.x * 3.1415 * 2;
	float v = vs_in_pos.y * 3.1415;

	vs_out_pos = vec3( 
			r * sin(v) * cos(u),
			r * cos(v),
			r * sin(v) * sin(u));

	vs_out_tex = vs_in_tex;
	vs_out_tex.y = 1 - vs_out_tex.y;
	vs_out_tex.x = 1 - vs_out_tex.x;

	vs_out_pos = vs_out_pos * (1.0 + length(texture(heightTexImage, vs_out_tex)) * 0.1);

	if(vs_in_pos.y > 0.01 && vs_in_pos.y < 0.99 && vs_in_pos.x > 0.01 && vs_in_pos.x < 0.99)
 	{

		vec3 du = GetPos(vs_in_pos.x, 0.01, vs_in_pos.y, 0.0, vs_out_tex) - GetPos(vs_in_pos.x, -0.01, vs_in_pos.y, 0.0, vs_out_tex);
		vec3 dv = GetPos(vs_in_pos.x, 0.0, vs_in_pos.y, 0.01, vs_out_tex) - GetPos(vs_in_pos.x, 0.0, vs_in_pos.y, -0.01, vs_out_tex);

		vs_out_norm = normalize(cross(du, dv));
	}
	else
	{
		vs_out_norm = vec3(
				 sin(v) * cos(u),
				 cos(v),
				 sin(v) * sin(u));
	}

	gl_Position = viewProj * world * vec4( vs_out_pos, 1 );
	vs_out_pos  = (world   * vec4(vs_out_pos,  1)).xyz;
	vs_out_norm = (worldIT * vec4(vs_out_norm, 0)).xyz;

}

vec3 GetPos(float u, float udif, float v, float vdif, vec2 temp_tex)
{
	vec2 tex = vec2(0,0);
	tex.x = temp_tex.x - udif;
	tex.y = temp_tex.y - vdif;

	u = u + udif;
	v = v + vdif;

	u = u * 3.1415 * 2;
	v = v * 3.1415;

	return vec3(sin(v) * cos(u),
				cos(v),
				sin(v) * sin(u)) *
				(1.0 + length(texture(heightTexImage, tex)) * 0.1) * -1.0;
}