#version 460 core

layout(std140, binding = 0) uniform CBPerFrame
{
    mat4 proj;
    mat4 projInverse;
    mat4 view;
    mat4 viewRotation;
    vec4 cameraPosition;

    int screenWidth;
    int screenHeight;
    float cameraNear;
    float cameraFar;

    int enableSSAO;
    float SSAODistance;
    float SSAODistancePower;
    float SSAOMoveAmount;

    vec3 sunDirection;
    float sunlightBias;

    float sunlightIntensity;
    float sunlightAmbient;
    float sunIndirectIntensity;
    int sunEnableShadow;

    int postEnableBloom;
    int postEnableToneMapping;
    float postSaturation;
    float postBloomIntensity;

    float postExposure;
    float postbBrightMax;
    float fogDensity;
    float metallic;

    float roughness;
    float pad1;
    float pad2;
    float pad3;
} cbPerFrame;

