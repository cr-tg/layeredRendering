#version 330 core
layout(location = 0) in vec2 postion;
layout(location = 1) in vec2 texcoords;

out vec2 TexCoords;

void main()
{
	gl_Position = vec4(postion,0.0,1.0);
	TexCoords = texcoords;
}