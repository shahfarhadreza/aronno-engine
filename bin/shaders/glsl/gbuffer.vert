#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

layout(std140, binding = 0) uniform CBPerFrame
{
    mat4 proj;
    mat4 view;
    mat4 world;
    mat4 lightviewproj;
    vec4 lightPosition;
    vec4 cameraPosition;
    float hasNormalMap;
    float enableSSAO;
    vec2 pad;
    vec4 pad2;
} cbPerFrame;


out vec3 fragmentPosition;
out vec3 fragmentNormal;

void main(){
    vec4 viewPos = cbPerFrame.view * cbPerFrame.world * vec4(position, 1.0);
	gl_Position = cbPerFrame.proj * viewPos;
	fragmentPosition = viewPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(cbPerFrame.view * cbPerFrame.world)));

    fragmentNormal = normalMatrix * normal;
}
