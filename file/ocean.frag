#version 430

#define ONE_OVER_4PI	0.0795774715459476

out vec4 my_FragColor0;

layout(binding = 1) uniform sampler2D perlin;
layout(binding = 2) uniform samplerCube envmap;

uniform vec4 uvParams;
uniform vec2 perlinOffset;
uniform vec3 oceanColor;

in vec3 vdir;
in vec2 tex;
in vec3 eye;
in vec3 local;

const vec3 sunColor			= vec3(1.0, 0.8, 0.27);
const vec3 perlinFrequency	= vec3(5.12, 1.59, 0.23);
const vec3 perlinGradient	= vec3(0.012, 0.016, 0.022);
const vec3 sundir			= vec3(0.603, 0.240, -0.761);
//const vec3 sundir			= vec3(5.603, 5.240, 5.761);

void main()
{
	


	float dist = length(vdir.xz);
	
	vec2 perl = vec2(0.0);
	
	vec2 ptex = tex + uvParams.zw;

	vec2 p0 = texture(perlin, ptex * perlinFrequency.x + perlinOffset).rg;
	vec2 p1 = texture(perlin, ptex * perlinFrequency.z + perlinOffset).rg;

	perl = (p0 * perlinGradient.x + p1 * perlinGradient.z) * 2;
	

	// calculate thingies
	vec4 grad = vec4(0, 0, 0.20, 1.0);
	grad.xy = perl;

	vec3 n = normalize(grad.xzy);
	vec3 v = normalize(vdir);
	vec3 l = reflect(-v, n);

	float F0 = 0.5;
	float F = F0 + (1.0 - F0) * pow(1.0 - dot(n, l), 5.0);

	vec3 refl = texture(envmap, l).rgb;

	// Ward model
	const float rho = 0.3;
	const float ax = 0.35;
	const float ay = 0.2;

	vec3 h = sundir + v;
	vec3 x = cross(sundir, n);
	vec3 y = cross(x, n);

	float mult = (ONE_OVER_4PI * rho / (ax * ay * sqrt(max(1e-5, dot(sundir, n) * dot(v, n)))));
	float hdotx = dot(h, x) / ax;
	float hdoty = dot(h, y) / ay;
	float hdotn = dot(h, n);

	float spec = mult * exp(-((hdotx * hdotx) + (hdoty * hdoty)) / (hdotn * hdotn));

	my_FragColor0 = vec4(mix(oceanColor, refl, F) + sunColor * spec * 0.5, 0.9);
}
