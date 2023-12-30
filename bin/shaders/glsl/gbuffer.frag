#version 460 core

in vec3 fragmentPosition;
in vec3 fragmentNormal;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;

layout(binding = 0) uniform sampler2D DiffuseTexture;

void main()
{      
    gPosition = fragmentPosition;
    gNormal = normalize(fragmentNormal);
} 

