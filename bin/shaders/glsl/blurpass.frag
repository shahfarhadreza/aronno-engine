

in vec2 textureCoordinate;

layout(binding = 0) uniform sampler2D brightPass;

layout(std140, binding = 5) uniform CBPostProcess
{
    float horizontalPass; // 0 or 1 to indicate vertical or horizontal pass
    vec3 pad;
} cbPostProcess;

out vec4 fragColor;

const int iRadius = 18;

float weight( int i ){
  return ((iRadius/2.0) - abs(i - iRadius/2.0))/(iRadius/2.0);
}

void main() {

    fragColor = vec4(0,0,0,0);
    ivec2 txsUV = ivec2(floor( textureCoordinate * textureSize( brightPass, 0 ) ));

    if (cbPostProcess.horizontalPass == 1) {
        for( int i = 0; i < iRadius; i++ ) {
            fragColor += texelFetch( brightPass, txsUV + ivec2(i - (iRadius/2), 0), 0 ) * ( weight(i) ) / iRadius;
        }
    }
    else {
        for( int i = 0; i < iRadius; i++ ){
            fragColor += texelFetch( brightPass, txsUV + ivec2(0, i - (iRadius/2)), 0 ) * ( weight(i) ) * 3.0 / iRadius;
        }
    }
}

