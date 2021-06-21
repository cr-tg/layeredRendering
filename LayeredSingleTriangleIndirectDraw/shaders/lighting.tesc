#version 440 core                                                                               
                                                                                                
// define the number of CPs in the output patch                                                 
layout (vertices = 3) out;                                                                      
                                                                                                
uniform vec3 gEyeWorldPos;                                                                      


in VS_OUT
{
    vec3 worldPos;
    vec3 normal;
    vec3 worldNormal;
    vec2 texCoord;
}tc_in[];

out VS_OUT
{
    vec3 worldPos;
    vec3 normal;
    vec3 worldNormal;
    vec2 texCoord;
}tc_out[];
                                                                                                
float GetTessLevel(float Distance0, float Distance1)                                            
{                                                                                               
     float AvgDistance = (Distance0 + Distance1) / 2.0;  
     float maxTessLevel = 20.0f;
     float tessLevel = 0.0f;
     tessLevel = floor(max(maxTessLevel - AvgDistance,1.0f)+0.5f);
     return tessLevel;
}                                                                                               
                                                                                                
void main()                                                                                     
{                                                                                               
    // Set the control points of the output patch                                               
    tc_out[gl_InvocationID].worldPos = tc_in[gl_InvocationID].worldPos;                          
    tc_out[gl_InvocationID].normal = tc_in[gl_InvocationID].normal;                          
    tc_out[gl_InvocationID].worldNormal = tc_in[gl_InvocationID].worldNormal;                          
    tc_out[gl_InvocationID].texCoord = tc_in[gl_InvocationID].texCoord;                          
                                                                                                
    // Calculate the distance from the camera to the three control points                       
    float EyeToVertexDistance0 = distance(gEyeWorldPos, tc_out[0].worldPos);                     
    float EyeToVertexDistance1 = distance(gEyeWorldPos, tc_out[1].worldPos);                     
    float EyeToVertexDistance2 = distance(gEyeWorldPos, tc_out[2].worldPos);                     
                                                                                                
    // Calculate the tessellation levels                                                        
    gl_TessLevelOuter[0] = GetTessLevel(EyeToVertexDistance1, EyeToVertexDistance2);            
    gl_TessLevelOuter[1] = GetTessLevel(EyeToVertexDistance2, EyeToVertexDistance0);            
    gl_TessLevelOuter[2] = GetTessLevel(EyeToVertexDistance0, EyeToVertexDistance1);            
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];                                                
}                                                               