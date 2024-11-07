#version 410 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 Color;
layout(location = 2) in vec2 UVIn;

uniform mat4 u_Model;

uniform mat4 u_View;

uniform mat4 u_Projection;

out vec2 UV;

void main()
{
	gl_Position = u_Projection * u_View * u_Model * vec4(position, 1.0);

	UV = UVIn;

}

