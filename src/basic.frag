#version 440

// Incoming vertex colour
layout (location = 0) in vec4 in_colour;

// Outgoing pixel colour
layout (location = 0) out vec4 out_colour;

uniform vec3 overrideColour;

void main()
{
	// Simply set outgoing colour
	out_colour = vec4(overrideColour,1.0f)*in_colour;
	
}