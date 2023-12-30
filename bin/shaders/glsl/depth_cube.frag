

layout(std140, binding = 3) uniform CBShadowCube
{
    mat4 shadowMatrices[6];
    vec4 lightPos;
} cbShadowCube;

in vec4 FragPos;

void main()
{      
    // Manually sets depth map in the range [0, 1]
    float farPlane = cbShadowCube.lightPos.w;
    gl_FragDepth = length(FragPos.xyz - cbShadowCube.lightPos.xyz) / farPlane;
} 

