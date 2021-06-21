#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out VS_OUT
{
    vec3 worldPos;
    vec3 normal;
    vec3 worldNormal;
}vs_out;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    vs_out.normal = vec3(projection * view * vec4(normalMatrix * aNormal, 0.0));
    vs_out.worldNormal = vec3(normalMatrix * aNormal);
    vs_out.worldPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}



