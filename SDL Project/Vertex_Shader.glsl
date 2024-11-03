#version 410 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 Color;
layout(location = 2) in vec2 UVIn;

out vec2 UV;

void main()
{
	gl_Position = vec4(position, 1.0);

	UV = UVIn;

}

