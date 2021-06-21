#version 430 core
layout (location = 0 ) in vec2 uv;
layout (location = 1 ) flat in uint drawID;

layout (location = 0) out vec4 color;

layout (binding = 0) uniform sampler2DArray textureArray;

struct DrawElementsCommand
{
	uint vertexCount;
	uint instanceCount;
	uint firstIndex;
	uint baseVertex;
	uint baseInstance;
};

layout(std430,binding = 0) coherent restrict buffer ssboLight
{
	DrawElementsCommand draw_commands[];
};

void main(void)
{
   color = texture(textureArray, vec3(uv.x, uv.y, drawID) );
}