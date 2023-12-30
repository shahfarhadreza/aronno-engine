

layout (location = 0) in vec3 position;

layout(std140, binding = 1) uniform CBPerObject
{
    mat4 world;
    float hasNormalMap;
    float hasEmissionMap;
    float opacity;
    float specularIntensity;
    float specularGlossiness;
    float emissionIntensity;
    float pad1;
    float pad2;
} cbPerObject;

out vec3 textureCoordinate;

void main()
{
    vec4 pos = cbPerFrame.proj * cbPerFrame.viewRotation * vec4(position, 1.0);
    pos.z += 1; // Offset to avoid z-fighting 
    gl_Position = pos;
	textureCoordinate = position;
} 

