#version 430

// NOTE: also defined in fragment shader
#define BLEND_START		8		// m
#define BLEND_END		200		// m

layout (location = 0) in vec3 my_Position;

layout(binding = 1) uniform sampler2D perlin;

uniform mat4 matLocal;
uniform mat4 matWorld;
uniform mat4 matViewProj;
uniform vec4 uvParams;
uniform vec2 perlinOffset;
uniform vec3 eyePos;

out vec3 vdir;
out vec2 tex;

void main()
{
	// transform to world space
	vec4 pos_local = matLocal * vec4(my_Position, 1.0);
	vec2 uv_local = pos_local.xz * uvParams.x + vec2(uvParams.z);

	vdir = eyePos - pos_local.xyz;
	tex = uv_local;

	gl_Position = matViewProj * vec4(pos_local.xyz, 1.0);
}
