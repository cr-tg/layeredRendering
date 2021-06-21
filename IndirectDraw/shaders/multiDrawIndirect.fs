#version 430 core
layout (binding=0) uniform sampler2DArray textureArray;
in vec2 uv;
flat in uint drawID;

out vec4 color;

void main(void)
{
    color = texture(textureArray, vec3(uv.x,uv.y,drawID) );
}