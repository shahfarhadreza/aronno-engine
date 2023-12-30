

layout (location = 0) in vec3 position;
layout (location = 3) in vec2 textureCoord;

layout(std140, binding = 1) uniform CBPerObject
{
    mat4 world;
    float hasNormalMap;
    float hasEmissionMap;
    float opacity;
    float specularIntensity;
    float specularGlossiness;
    float emissionIntensity;
    float animated;
    float isTransparent;
} cbPerObject;

out vec2 textureCoordinate;

void main(){
	gl_Position = cbPerFrame.proj * cbPerObject.world * vec4(position, 1.0);
	textureCoordinate = textureCoord ;
}
