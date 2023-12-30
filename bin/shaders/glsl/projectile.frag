

in vec2 textureCoordinate;

layout(binding = 3) uniform sampler2D gColor;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragBrightness;

vec4 getBrightness(vec4 color) {
    vec3 luminanceVector = vec3(0.2126, 0.7152, 0.0722);

    float brightness = dot(luminanceVector, color.xyz);

    if (brightness > 0.8) {
        return color;
    } else {
        return vec4(0.0, 0.0, 0.0, 1.0);
    }
}

void main() {
    vec4 color = texture(gColor, textureCoordinate).rgba;

    //if (color.a < 0.5) 
      // discard;

    fragColor = color;
    fragBrightness = getBrightness(fragColor);
}

