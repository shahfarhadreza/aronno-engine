

in vec2 textureCoordinate;

layout(binding = 1) uniform sampler2D diffuseMap;

layout(location = 0) out float fragmentdepth;

void main()
{     
    vec4 color = texture(diffuseMap, textureCoordinate);
    if(color.a < 0.3)
        discard;   
      
    fragmentdepth = gl_FragCoord.z;
} 

