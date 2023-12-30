

in vec2 textureCoordinate;

layout(binding = 4) uniform sampler2D depthTexture;
layout(binding = 5) uniform sampler2D noiseTexture;

const int samples = 32;

layout(std140, binding = 4) uniform CBSSAO {
    vec4 sample_sphere[samples];
} cbSSAO;

// source: https://github.com/N8python/ssao/blob/master/EffectShader.js#L52
vec3 getWorldPos(const float depth, const vec2 coord) {
    float z = depth * 2.0 - 1.0;
    vec4 clipSpacePosition = vec4(coord * 2.0 - 1.0, z, 1.0);

    vec4 viewSpacePosition = cbPerFrame.projInverse * clipSpacePosition;

    // Perspective division
    vec4 worldSpacePosition = /*cameraMatrixWorld * */viewSpacePosition;
    worldSpacePosition.xyz /= worldSpacePosition.w;

    return worldSpacePosition.xyz;
}

vec3 computeNormal(vec3 worldPos, vec2 vUv) {
    vec2 size = vec2(textureSize(depthTexture, 0));
    ivec2 p = ivec2(vUv * size);
    float c0 = texelFetch(depthTexture, p, 0).x;
    float l2 = texelFetch(depthTexture, p - ivec2(2, 0), 0).x;
    float l1 = texelFetch(depthTexture, p - ivec2(1, 0), 0).x;
    float r1 = texelFetch(depthTexture, p + ivec2(1, 0), 0).x;
    float r2 = texelFetch(depthTexture, p + ivec2(2, 0), 0).x;
    float b2 = texelFetch(depthTexture, p - ivec2(0, 2), 0).x;
    float b1 = texelFetch(depthTexture, p - ivec2(0, 1), 0).x;
    float t1 = texelFetch(depthTexture, p + ivec2(0, 1), 0).x;
    float t2 = texelFetch(depthTexture, p + ivec2(0, 2), 0).x;
    float dl = abs((2.0 * l1 - l2) - c0);
    float dr = abs((2.0 * r1 - r2) - c0);
    float db = abs((2.0 * b1 - b2) - c0);
    float dt = abs((2.0 * t1 - t2) - c0);
    vec3 ce = getWorldPos(c0, vUv).xyz;
    vec3 dpdx = (dl < dr) ? ce - getWorldPos(l1, (vUv - vec2(1.0 / size.x, 0.0))).xyz
                          : -ce + getWorldPos(r1, (vUv + vec2(1.0 / size.x, 0.0))).xyz;
    vec3 dpdy = (db < dt) ? ce - getWorldPos(b1, (vUv - vec2(0.0, 1.0 / size.y))).xyz
                          : -ce + getWorldPos(t1, (vUv + vec2(0.0, 1.0 / size.y))).xyz;
    return normalize(cross(dpdx, dpdy));
}

highp float linearize_depth(highp float d, highp float zNear, highp float zFar) {
    highp float z_n = 2.0 * d - 1.0;
    return 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}

void main() {

    float depth = texture(depthTexture, textureCoordinate).r;

    vec3 worldPos = getWorldPos(depth, textureCoordinate);
    vec3 normal = computeNormal(worldPos, textureCoordinate);

    vec2 noiseScale = vec2(float(cbPerFrame.screenWidth)/4.0, float(cbPerFrame.screenHeight)/4.0);

    vec3 randomVec = normalize(texture(noiseTexture, textureCoordinate * noiseScale).xyz * 2.0 - 1.0);
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal);

    float occluded = 0.0;
    float totalWeight = 0.0;

    vec3 samplePos;

    float sppF = float(samples);

    float cameraNear = cbPerFrame.cameraNear;
    float cameraFar = cbPerFrame.cameraFar;

    float aoDistance = cbPerFrame.SSAODistance;//1.0f;
    float distancePower = cbPerFrame.SSAODistancePower;//0.950f;
    float moveAmt = cbPerFrame.SSAOMoveAmount;//2.5f;

    for (float i = 0.0; i < sppF; i++) {
        vec3 sampleDirection = tbn * cbSSAO.sample_sphere[int(i)].xyz;

        // make sure sample direction is in the same hemisphere as the normal
        if (dot(sampleDirection, normal) < 0.0) 
          sampleDirection *= -1.0;

        //float moveAmt = samplesR[int(mod(i + noise.a * sppF, sppF))];
        samplePos = worldPos + aoDistance * moveAmt * sampleDirection;

        vec4 offset = cbPerFrame.proj * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sampleDepth = texture(depthTexture, offset.xy).r;

        float distSample = linearize_depth(sampleDepth, cameraNear, cameraFar);
        float distWorld = linearize_depth(offset.z, cameraNear, cameraFar);

        float rangeCheck = smoothstep(0.0, 1.0, aoDistance / (aoDistance * abs(distSample - distWorld)));
        rangeCheck = pow(rangeCheck, distancePower);
        float weight = dot(sampleDirection, normal);

        occluded += rangeCheck * weight * (distSample < distWorld ? 1.0 : 0.0);
        totalWeight += weight;
    }

    float occ = clamp(1.0 - occluded / totalWeight, 0.0, 1.0);

    float ao = pow(occ, 5);

    gl_FragColor = vec4(ao);
}

