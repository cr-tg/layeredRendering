#version 460 core
layout(early_fragment_tests) in;//������ǰ��Ȳ��ԣ�������ɼ���ֱ�ӱ�discard������������Ƭ����ɫ��
//layout(pixel_interlock_unordered) in; //��fs��Ƭ�εĴ�����Ҫ����ͼԪ��˳��
//�ؼ��ֶ���Ч�ʵ�Ӱ�첻����Ҫ��coherent�ؼ��ֶ���imageͬ���ȽϹؼ�
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
	uint currentIndex = atomicCounterIncrement(uListNodeCounter);//currentIndex��1��ʼ
	if (currentIndex > uMaxListNode)
	{
		discard;//����ֻ��ʹ��discard��openGL��������return�Ժ�call begin()/end()
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

