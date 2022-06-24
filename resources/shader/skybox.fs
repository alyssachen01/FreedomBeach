#version 330 core
out vec4 FragColor;

in vec3 TexCoords;
in float time;

struct Light { 
    vec3 ambient;
};

uniform Light light;

uniform vec4 fogcolor;

uniform samplerCube skybox;

void main()
{    
    vec3 ambient = light.ambient * texture(skybox, TexCoords).rgb;

	float fogcoord = (gl_FragCoord.z/gl_FragCoord.w) * 200;
    //vec4 fogcolor = vec4(1, 1, 1, 0.5);            
    float Fog = exp2( - 0.000008 * fogcoord * fogcoord * time); 
    Fog = clamp(Fog, 0.0, 1.0); 
    vec4 color = vec4(ambient, 1.0);

    if (time == 0)
    {
      FragColor = color;
    }
    if (time > 0)
    {
      FragColor = mix(fogcolor, color, Fog);    
    }
}
