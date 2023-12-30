

in vec2 textureCoordinate;

layout(binding = 0) uniform sampler2D primaryPass;
layout(binding = 1) uniform sampler2D blurPass;
layout(binding = 2) uniform sampler2D ssaoPass;
layout(binding = 3) uniform sampler2D blackPass;

vec3 czm_saturation(vec3 rgb, float adjustment)
{
    // Algorithm from Chapter 16 of OpenGL Shading Language
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    vec3 intensity = vec3(dot(rgb, W));
    return mix(intensity, rgb, adjustment);
}

void main() {

    //vec4 blackColor = texture(blackPass, textureCoordinate).rgba;
    //gl_FragColor = vec4(blackColor);

    vec4 primaryColor = texture(primaryPass, textureCoordinate).rgba;

    vec3 hdrColor = primaryColor.rgb;

    if (cbPerFrame.postEnableBloom == 1) {
        // How much bloom to add
        const float bloomFactor = cbPerFrame.postBloomIntensity;

        vec3 bloomColor = texture(blurPass, textureCoordinate).rgb;

        hdrColor += bloomColor * bloomFactor;
    }

    float ssao = 1.0f;

    if (cbPerFrame.enableSSAO == 1) {
        vec2 texelSize = 1.0 / vec2(textureSize(ssaoPass, 0));
        float result = 0.0;
        for (int x = -2; x < 2; ++x) 
        {
            for (int y = -2; y < 2; ++y) 
            {
                vec2 offset = vec2(float(x), float(y)) * texelSize;
                result += texture(ssaoPass, textureCoordinate + offset).r; 
            }
        }

        ssao = result / 16;

        hdrColor *= ssao;
    }

    if (cbPerFrame.postEnableToneMapping == 1) {
        const float exposure = cbPerFrame.postExposure;
        const float brightMax = cbPerFrame.postbBrightMax;

        // Perform tone-mapping
        float Y = dot(vec3(0.30, 0.59, 0.11), hdrColor);
        float YD = exposure * (exposure/brightMax + 1.0) / (exposure + 1.0);
        hdrColor *= YD;
    }

    // tone mapping
    vec3 mapped = hdrColor;

    mapped = czm_saturation(mapped, cbPerFrame.postSaturation);

     // apply gamma correction
    float gamma = 2.2;
    mapped = pow(mapped, vec3(1.0/gamma));

    gl_FragColor = vec4(mapped, primaryColor.a);
}

