#version 330

uniform sampler2D sampler_tex;

uniform vec4 diffuse_colour;

in vec2 varying_colour;

out vec4 fragment_colour;

void main(void)
{
	//vec3 jeep_colour = texture(sampler_tex, varying_colour).rgb;
	//fragment_colour = vec4(jeep_colour,1.0);
	fragment_colour = vec4(1.0, 1.0, 1.0, 1.0);
}