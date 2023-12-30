
const int MAX_BONE_INFLUENCE = 8;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 textureCoord;

layout (location = 4) in vec4 BoneIDs;
layout (location = 5) in vec4 BoneIDs2;
layout (location = 6) in vec4 Weights;
layout (location = 7) in vec4 Weights2;

out VS_OUT {
	vec2 textureCoordinate;
	vec3 normal; // World/Model space
	vec3 tangent; // World/Model space
	vec3 fragPos; // World/Model space
    vec4 fragViewPos; // View space
} vs_out;

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

const int MAX_BONES = 200;

layout(std140, binding = 8) uniform CBPerAnimatedObject
{
    mat4 gBones[MAX_BONES];
} cbPerAnimatedObject;

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
        mat4 invTrans = transpose(inverse(BoneTransform));

        vec4 localNormal = invTrans * vec4(aNormal, 1.0);
        vec4 localTangent = invTrans * vec4(aTangent, 1.0);

        gl_Position = cbPerFrame.proj * cbPerFrame.view * cbPerObject.world * totalPosition;

        vs_out.textureCoordinate = textureCoord;

        mat4 wInv = transpose(inverse(cbPerObject.world));

        vs_out.fragPos = vec3(cbPerObject.world * totalPosition);
        vs_out.tangent = vec3(wInv * localTangent);

        if (cbPerObject.hasNormalMap == 1) {
            vs_out.normal = vec3(wInv * localNormal);
        } else {
            vs_out.normal = vec3(wInv * localNormal);
        }

    } else {

        gl_Position = cbPerFrame.proj * cbPerFrame.view * cbPerObject.world * vec4(position, 1.0);

        vs_out.textureCoordinate = textureCoord;

        vs_out.fragPos = vec3(cbPerObject.world * vec4(position, 1.0));
        vs_out.tangent = vec3(cbPerObject.world * vec4(aTangent, 0.0));

	    mat4 wInv = transpose(inverse(cbPerObject.world));

        if (cbPerObject.hasNormalMap == 1) {
            vs_out.normal = vec3(wInv * vec4(aNormal, 0.0));
        } else {
            vs_out.normal = vec3(wInv * vec4(aNormal, 1.0));
        }
    }

    vs_out.fragViewPos = cbPerFrame.view * vec4(vs_out.fragPos, 1.0);
}

