#version 430 core
out vec4 color;
in vec2 uv;
layout (binding=0) uniform sampler2DArray textureArray;
layout (location=1) uniform int layer;
                  
void main(void)
{
     color = texture(textureArray, vec3(uv.x,uv.y,layer) );
}