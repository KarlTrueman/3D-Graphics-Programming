#version 330

uniform sampler2D sampler_tex;

uniform vec4 diffuse_colour;

in vec2 varying_coord;
in vec3 varying_normal;
in vec3 varying_position;

out vec4 fragment_colour;

void main(void)
{
	vec3 tex_colour = texture(sampler_tex, varying_coord).rgb;
	fragment_colour = vec4(tex_colour,1.0);
	//fragment_colour = vec4(1.0, 0.5, 1.0, 1.0);
	vec3 N = normalize(varying_normal);
}