#version 440 core                                                                               
                                                                                                
layout(triangles, equal_spacing, ccw) in;                                                       
layout(binding = 0) uniform sampler2D gDisplacementMap;                                                             
                                                                                                
uniform mat4 matVP;                                                                               
uniform float gDispFactor;  

in VS_OUT
{
    vec3 worldPos;
    vec3 normal;
    vec3 worldNormal;
    vec2 texCoord;
}te_in[];

out VS_OUT
{
    vec3 worldPos;
    vec3 normal;
    vec3 worldNormal;
    vec2 texCoord;
}te_out;

out vec4 fcolor;

float rand(float seed) {
    return fract(sin(seed)*100000.0);
}

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)                                                   
{                                                                                               
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;   
}                                                                                               
                                                                                                
vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)                                                   
{                                                                                               
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;   
}                                                                                               
                                                                                                
void main()                                                                                     
{                                                                                               
    // Interpolate the attributes of the output vertex using the barycentric coordinates        
    te_out.normal = interpolate3D(te_in[0].normal, te_in[1].normal, te_in[2].normal);            
    te_out.normal = normalize(te_out.normal);  
    te_out.worldNormal = interpolate3D(te_in[0].worldNormal, te_in[1].worldNormal, te_in[2].worldNormal);            
    te_out.worldNormal = normalize(te_out.worldNormal);  
    te_out.worldPos = interpolate3D(te_in[0].worldPos, te_in[1].worldPos, te_in[2].worldPos);            
    te_out.texCoord = interpolate2D(te_in[0].texCoord, te_in[1].texCoord, te_in[2].texCoord);    
                                                                                                
    // Displace the vertex along the normal
    float Displacement = texture(gDisplacementMap, te_out.texCoord).x;                        
    te_out.worldPos += te_out.worldNormal * gDispFactor * Displacement;
    float randomNum = rand(Displacement);
    fcolor = vec4(0.0f,randomNum,0.8f,1.0f);

//    if(randomNum < 0.33f)
//        fcolor = vec4(1.0f,0.0f,0.0f,1.0f);
//    else if(randomNum < 0.67f)
//        fcolor = vec4(0.0f,1.0f,0.0f,1.0f);
//    else 
//        fcolor = vec4(0.0f,0.0f,1.0f,1.0f);
    gl_Position = matVP * vec4(te_out.worldPos, 1.0);                                              
}                                                                                               
