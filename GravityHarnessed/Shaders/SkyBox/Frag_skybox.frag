#version 430

// pipeline-ból bejövő per-fragment attribútumok
in vec3 vs_out_pos;

layout (location = 0) out vec4 fs_out_col;
layout (location = 1) out vec4 fs_out_justSun;

uniform vec3 groundColor = vec3(0.4,0.3,0.2);
uniform vec3 skyColor = vec3(0.2,0.2,1.0);

// skybox textúra
uniform samplerCube skyboxTexture;


void main()
{
	//fs_out_col = vec4( vec3(vs_out_pos.y),1.0);
	//fs_out_col = vec4( mix(groundColor, skyColor, normalize(vs_out_pos).y *0.5 + 0.5),1.0);

	//fs_out_col = vec4(texture(skyboxTexture, normalize(vs_out_pos)).xyz, 1);
	fs_out_col = texture( skyboxTexture, vs_out_pos );
	fs_out_justSun = vec4(0.0,0.0,0.0,1.0);
}