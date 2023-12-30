#version 330 core

in vec2 textureCoordinate;
in vec3 Normal;
in vec3 fragmentPosition;

// object material
uniform vec3 diffuseColor;
uniform vec3 specularColor;

// light
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform mat4 lightSpaceMatrix;

// more info
uniform vec3 viewPos;

uniform sampler2D diffuseMap;
uniform sampler2DShadow shadowMap;

out vec4 fragColor;

const float PI = 3.141592;
const float Epsilon = 0.00001;

vec3 ShadowCoords(mat4 shadowMapViewProjection)
{
	vec4 fragPosLightSpace = shadowMapViewProjection * vec4(fragmentPosition, 1.0);
	// perform perspective divide
    vec3 shadowCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    shadowCoords = shadowCoords * 0.5 + 0.5;
	return shadowCoords;
}

float ShadowCalculation() {
    vec3 projCoords = ShadowCoords(lightSpaceMatrix);

    float EPSILON = 0.00001;

    float xOffset = 1.0/2048.0;
    float yOffset = 1.0/2048.0;

    float Factor = 0.0;

    for (int y = -1 ; y <= 1 ; y++) {
        for (int x = -1 ; x <= 1 ; x++) {
            vec2 Offsets = vec2(x * xOffset, y * yOffset);
            vec3 UVC = vec3(projCoords.xy + Offsets, projCoords.z + EPSILON);
            Factor += texture(shadowMap, UVC);
        }
    }
    float shadow = (0.5 + (Factor / 18.0));
    /*
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // check whether current frag pos is in shadow
	float biasedDepth = currentDepth - bias;
    float shadow = biasedDepth > closestDepth  ? 1.0 : 0.0;
    if (projCoords.z > 1.0)
        shadow = 0.0;
    */
    return shadow;
}

void main() {
  	vec3 color = texture(diffuseMap, textureCoordinate).xyz;

	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(lightPos - fragmentPosition);

	float NdotL = dot(normal, lightDir);

  	// diffuse
	vec3 diffuse = max(NdotL, 0.0) * lightColor;

	// specular
    vec3 viewDir = normalize(viewPos - fragmentPosition);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	float shininess = 16.0f;

    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = (spec * lightColor) * specularColor;

	// ambient
	float ambientStrength = 0.5;
	vec3 ambient = ambientStrength * lightColor;

    //float bias = max(0.05 * (1.0 - NdotL), 0.005);

    float shadow = 0.0f;

	// calculate shadow
    shadow = ShadowCalculation();

    vec3 lighting = (ambient + diffuse + specular) * shadow * color;


    fragColor = vec4(lighting, 1.0);
}


