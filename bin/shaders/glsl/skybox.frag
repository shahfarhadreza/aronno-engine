

in vec3 textureCoordinate;

layout(binding = 3) uniform samplerCube skybox;

vec3 adjustSaturation(vec3 color, float value) {
  // https://www.w3.org/TR/WCAG21/#dfn-relative-luminance
  const vec3 luminosityFactor = vec3(0.2126, 0.7152, 0.0722);
  vec3 grayscale = vec3(dot(color, luminosityFactor));

  return mix(grayscale, color, 1.0 + value);
}

out vec4 FragColor;

void main()
{    
    vec3 color = texture(skybox, textureCoordinate).rgb;

    //color = adjustSaturation(color, 1.1);

    FragColor = vec4(color, 1.0);
}

