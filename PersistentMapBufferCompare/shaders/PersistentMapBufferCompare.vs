#version 440 core
layout (location = 0 ) in vec2 position;

uniform float uAngle;

void main(void)
{
    vec2 pos;
    pos.x = position.x * cos(uAngle) - position.y * sin(uAngle);
	pos.y = position.x * sin(uAngle) + position.y * cos(uAngle);
    gl_Position = vec4(pos,0.0,1.0);
} 