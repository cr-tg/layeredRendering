#version 440 core  
layout (triangles) in;
//layout (triangle_strip, max_vertices = 3) out;
layout (triangle_strip, max_vertices = 39) out;
layout(binding = 0) uniform sampler2D noiseTex;

uniform float time;
uniform bool offset;
uniform vec3 cameraPos;

in VS_OUT
{
    vec3 worldPos;
    vec3 normal;
    vec3 worldNormal;
    vec2 texCoord;
}gs_in[];

out vec4 fcolor;
flat out uint layerNum;

float random(vec3 st) {
    return fract(sin(dot(st, vec3(53.81125,6.42648,24.40232)))*73424.64639);
}

vec4 explode(vec4 position, vec3 normal,int times)
{
    float magnitude = 0.02 * times;//0.005
    float dir = -1.0;
    if(!offset)
        magnitude = 0.0;
    //vec3 direction = dir*normal * ((sin(time) + 1.0) / 2.0) * magnitude;
   // normal +=1.0*vec3(random(vec3(normal*times+0.5)));
    vec3 randomNum = texture(noiseTex,gs_in[0].texCoord).rgb;
    normal = normalize(normal);
    normal += randomNum;
    //normal += vec3(random(vec3(times)),random(vec3(times*2)),random(vec3(times*3)));
    normal = normalize(normal);
    vec3 direction = dir * normal * magnitude;
    return position - vec4(direction.xy,abs(direction.z), 0.0);
}

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

void GenerateLine(int index)
{
    float magnitude = 0.3;
    gl_Position = gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * magnitude;
    EmitVertex();
    EndPrimitive();
}

void generateFirstLayerTriangle(int times)
{
     int index = 0;
     fcolor = vec4(1.0,0.0,0.0,0.5);
     gl_Position = explode(gl_in[index].gl_Position, gs_in[index].normal,times);
     //gl_Position = explode(gl_in[index].gl_Position, gs_in[index].worldNormal,times);
     EmitVertex();
     index = 1;
     gl_Position = explode(gl_in[index].gl_Position, gs_in[index].normal,times);
     //gl_Position = explode(gl_in[index].gl_Position, gs_in[index].worldNormal,times);
     EmitVertex();
     index = 2;
     gl_Position = explode(gl_in[index].gl_Position, gs_in[index].normal,times);
     //gl_Position = explode(gl_in[index].gl_Position, gs_in[index].worldNormal,times);
     EmitVertex();
}

void generateSecondLayerTriangle(int times)
{
     int index = 1;
     fcolor = vec4(0.0,1.0,0.0,0.5);
     gl_Position = explode(gl_in[index].gl_Position, gs_in[index].normal,times);
     //gl_Position = explode(gl_in[index].gl_Position, gs_in[index].worldNormal,times);
     EmitVertex();
     index = 2;
     gl_Position = explode(gl_in[index].gl_Position, gs_in[index].normal,times);
     //gl_Position = explode(gl_in[index].gl_Position, gs_in[index].worldNormal,times);
     EmitVertex();
     index = 0;
     gl_Position = explode(gl_in[index].gl_Position, gs_in[index].normal,times);
     //gl_Position = explode(gl_in[index].gl_Position, gs_in[index].worldNormal,times);
     EmitVertex();
}

void generateThirdLayerTriangle(int times)
{
     int index = 2;
     fcolor = vec4(0.0,0.0,1.0,0.5);
     gl_Position = explode(gl_in[index].gl_Position, gs_in[index].normal,times);
     //gl_Position = explode(gl_in[index].gl_Position, gs_in[index].worldNormal,times);
     EmitVertex();
     index = 0;
     gl_Position = explode(gl_in[index].gl_Position, gs_in[index].normal,times);
     //gl_Position = explode(gl_in[index].gl_Position, gs_in[index].worldNormal,times);
     EmitVertex();
     index = 1;
     gl_Position = explode(gl_in[index].gl_Position, gs_in[index].normal,times);
     //gl_Position = explode(gl_in[index].gl_Position, gs_in[index].worldNormal,times);
     EmitVertex();
}

void generateTriangle(int times)
{
    int mode = times % 3;
    switch(mode)
    {
        case 0:
        generateFirstLayerTriangle(times);
        break;
        case 1:
        generateSecondLayerTriangle(times);
        break;
        case 2:
        generateThirdLayerTriangle(times);
        break;
    }
}

void main() 
{
     vec3 triangleWorldPos = (gs_in[0].worldPos+gs_in[1].worldPos+gs_in[2].worldPos)/vec3(3.0f);
     vec3 triangleWorldNormal = normalize((gs_in[0].worldNormal+gs_in[1].worldNormal+gs_in[2].worldNormal)/vec3(3.0f));
     vec3 viewDir = normalize(cameraPos-triangleWorldPos);
     float nDotL = abs(dot(viewDir,triangleWorldNormal));


     float dis = distance(cameraPos,triangleWorldPos);
     uint maxLayer = uint(13);
     layerNum = max(maxLayer - uint(floor(dis)),uint(1));
     layerNum = uint(layerNum * (1.0-0.6*nDotL));

     generateTriangle(0);
     int times = 1;
     while(times < int(layerNum))
     {
        generateTriangle(times++);
     }
     EndPrimitive();
}







