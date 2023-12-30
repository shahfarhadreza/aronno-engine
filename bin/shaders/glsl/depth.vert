

layout (location = 0) in vec3 position;
layout (location = 3) in vec2 textureCoord;

layout (location = 4) in vec4 BoneIDs;
layout (location = 5) in vec4 BoneIDs2;
layout (location = 6) in vec4 Weights;
layout (location = 7) in vec4 Weights2;

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
    float pad2;
} cbPerObject;

const int MAX_BONES = 200;

layout(std140, binding = 8) uniform CBPerAnimatedObject
{
    mat4 gBones[MAX_BONES];
} cbPerAnimatedObject;

out vec2 textureCoordinate;

void main() {
    if (cbPerObject.animated == 1) {
        mat4 BoneTransform = mat4(0.0);

        BoneTransform += cbPerAnimatedObject.gBones[int(BoneIDs[0])] * Weights[0];
        BoneTransform += cbPerAnimatedObject.gBones[int(BoneIDs[1])] * Weights[1];
        BoneTransform += cbPerAnimatedObject.gBones[int(BoneIDs[2])] * Weights[2];
        BoneTransform += cbPerAnimatedObject.gBones[int(BoneIDs[3])] * Weights[3];

        BoneTransform += cbPerAnimatedObject.gBones[int(BoneIDs2[0])] * Weights2[0];
        BoneTransform += cbPerAnimatedObject.gBones[int(BoneIDs2[1])] * Weights2[1];
        BoneTransform += cbPerAnimatedObject.gBones[int(BoneIDs2[2])] * Weights2[2];
        BoneTransform += cbPerAnimatedObject.gBones[int(BoneIDs2[3])] * Weights2[3];

        vec4 totalPosition = BoneTransform * vec4(position, 1.0);

        gl_Position = cbPerFrame.proj * cbPerFrame.view * cbPerObject.world * totalPosition;
    } else {
        gl_Position = cbPerFrame.proj * cbPerFrame.view * cbPerObject.world * vec4(position, 1.0);
    }
	textureCoordinate = textureCoord;
}
