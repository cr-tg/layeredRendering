 
 #version 460 core
layout(location = 0) out vec4 outColor;


//layout(binding = 0,rgba32ui) uniform uimageBuffer linkedListsT;//GL_TEXTURE_BUFFER
layout(binding = 0,r32ui) uniform uimage2D uListHeadPtrTex;

struct ListNodeCam
{
	uvec3 nodeDataCam;//x存储next指针，y分量存储颜色，z存储深度(需要从浮点类型转换为int类型)
};


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

uniform vec2 iResolution;
uniform int uLayerNum;

#define MAX_FRAGMENTS 15
ListNode fragments[MAX_FRAGMENTS];

#define BACKCOLOR vec3(0.1f)

int buildLocalFragmentList();//将当前像素下的所有片元保存到数组中，返回这个list的长度
void bubbleSortFragmentList(int fragCount);
void insertSortFragmentList(int fragCount);
vec4 calFinalColor(int fragCount);
vec4 blend(vec4 preColor,vec4 currentColor);

//#define FROM_SCREEN(uv)  ( ( uv - iResolution.xy *.5 ) / iResolution.y * 2. )
#define FROM_SCREEN(uv)  ( ( uv - iResolution.xy *.9 ) / iResolution.y * 16. )
#define FROM_SCREEN1(uv)  ( ( uv - iResolution.xy *vec2(.85,.9) ) / iResolution.y * 16. )
const int CHARACTERS[14] = int[14](31599,9362,31183,31207,23524,29671,29679,30994,31727,31719,1488,448,2,3640);
float digitIsOn( int digit, vec2 id ) {   
    if ( id.x < .0 || id.y < .0 || id.x > 2. || id.y > 4. ) return .0;
    return floor( mod( float( CHARACTERS[ int( digit ) ] ) / pow( 2., id.x + id.y * 3. ), 2. ) );
}

vec3 showcase(int layerNum, in vec2 uv ) {
    vec2 id = floor( ( uv + vec2( .5, 1. ) ) * 2.5 );    
    return vec3( digitIsOn( int( layerNum ), id ) );
}


void main()
{
	int fragCount = 0;
	fragCount = buildLocalFragmentList();
	//bubbleSortFragmentList(fragCount);
	insertSortFragmentList(fragCount);
	outColor = calFinalColor(fragCount);
	if(uLayerNum < 10)
	{
		vec2 uv = FROM_SCREEN(gl_FragCoord.xy);
		vec3 color = showcase(uLayerNum,uv);
		outColor += vec4(color,0.0);
	}
	else
	{
		vec2 uv1 = FROM_SCREEN1(gl_FragCoord.xy);
		vec3 color = showcase(1,uv1);
		outColor += vec4(color,0.0);
		
		vec2 uv = FROM_SCREEN(gl_FragCoord.xy);
		color = showcase(uLayerNum-10,uv);
		outColor += vec4(color,0.0);
	}

}




int buildLocalFragmentList()
{
	int fragCount = 0;
	uint curretIndex = uint(imageLoad(uListHeadPtrTex,ivec2(gl_FragCoord.xy)).r);
	while(curretIndex != 0 && fragCount < MAX_FRAGMENTS)
	{
		ListNode node = nodeSet[curretIndex];
		fragments[fragCount] = node;
		fragCount++;
		curretIndex = node.next;
	}
	return fragCount;
}

void bubbleSortFragmentList(int fragCount)//将深度从小到大排列起来
{
	int i,j;
	for(int i = 0;i<fragCount;++i)
	{
		bool isInvert = false;
		//float depth_i = uintBitsToFloat(fragments[i].z);
		for(j = 0;j<fragCount-1-i;++j)
		{
			float depth_j = uintBitsToFloat(fragments[j].depth);
			float depth_j1 = uintBitsToFloat(fragments[j+1].depth);
			if(depth_j > depth_j1)//将i放到后面去
			{
				isInvert = true;
				ListNode temp = fragments[j];
				fragments[j] = fragments[j+1];
				fragments[j+1] = temp;
			}
		}
		if(!isInvert)
			break;
	}
}

void insertSortFragmentList(int fragCount)//插入排序
{
	int i,j;
	for(int i = 0;i<fragCount-1;++i)
	{
		for(j = i+1;j>0;--j)
		{
			if(uintBitsToFloat(fragments[j].depth) < uintBitsToFloat(fragments[j-1].depth))//新来的数更小，就与前面的数交换
			{
				ListNode temp = fragments[j-1];
				fragments[j-1] = fragments[j];
				fragments[j] = temp;
			}
			else break;//比前面的数更大，就证明整个序列是有序的
		}
	}
}

vec4 calFinalColor(int fragCount)
{
	if(fragCount == 0)
		return vec4(vec3(0.0),1.0);//背景的颜色
	vec4 preColor = unpackUnorm4x8(fragments[fragCount-1].color);//最后一层的颜色
	for(int i=fragCount-2;i>-1;--i)
	{
		vec4 currentColor = unpackUnorm4x8(fragments[i].color);//每个分量就是用f/255.0
		preColor = blend(preColor,currentColor);
	}
	return preColor;
}

vec4 blend(vec4 preColor,vec4 currentColor)
{
	vec4 blendColor;
	blendColor = vec4(currentColor.rgb*currentColor.a+preColor.rgb*(1-currentColor.a),currentColor.a);
	return blendColor;
}

