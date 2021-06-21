#version 460 core
layout(early_fragment_tests) in;//开启提前深度测试，如果不可见就直接被discard，并不会运行片段着色器
//layout(pixel_interlock_unordered) in; //让fs对片段的处理不需要按照图元的顺序
//关键字对于效率的影响不大，主要是coherent关键字对于image同步比较关键
layout(binding = 0, offset = 0) uniform atomic_uint uListNodeCounter;
layout(binding = 0,r32ui) uniform coherent restrict uimage2D uLightHeadPtrTex;//GL_TEXTURE_2D
layout(binding = 1,r32ui) uniform coherent restrict uimage2D uLayerNumImage;//GL_TEXTURE_2D

struct ListNode
{
	uint next;
	uint color;
	uint depth;
	uint toUsed;
};

layout(std430,binding = 0) coherent restrict buffer ssboLight
{
	ListNode nodeSet[];
};

uniform int uMaxListNode;
uniform vec4 uObjColor;

in vec4 fcolor;
flat in uint layerNum;

out vec4 testColor;

void main()
{
	uint currentIndex = atomicCounterIncrement(uListNodeCounter);//currentIndex从1开始
	if (currentIndex > uMaxListNode)
	{
		discard;//这里只能使用discard，openGL不允许在return以后call begin()/end()
	}
	uint oldIndex = imageAtomicExchange(uLightHeadPtrTex, ivec2(gl_FragCoord.xy), currentIndex);
	ListNode node;
	node.next = oldIndex;
	node.color = packUnorm4x8(fcolor);
	//node.color = packUnorm4x8(uObjColor);
	node.depth = floatBitsToUint(gl_FragCoord.z);
	node.toUsed = layerNum;
	nodeSet[currentIndex] = node;
	testColor = fcolor;
	imageStore(uLayerNumImage,ivec2(0), ivec4(layerNum));
	imageStore(uLayerNumImage,ivec2(gl_FragCoord.xy), ivec4(layerNum));
}

