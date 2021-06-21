#version 430 core
layout (location = 0 ) in vec2 position;
layout (location = 1 ) in vec2 texCoord;
layout (location = 2 ) in uint drawid;

out vec2 uv;
flat out uint drawID;

void main(void)
{
    gl_Position = vec4(position,0.0,1.0);
    uv = texCoord;
    drawID = drawid;
    //drawID = gl_InstanceID;
}