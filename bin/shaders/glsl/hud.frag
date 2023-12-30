
in vec2 textureCoordinate;

layout(binding = 3) uniform sampler2D gColor;

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

void main() {
    vec4 color = texture(gColor, textureCoordinate).rgba;

    if (color.a < 0.1) 
       discard;

    gl_FragColor = vec4(color.rgb, color.a * cbPerObject.opacity);
}

