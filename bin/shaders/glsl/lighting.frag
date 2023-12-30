
in VS_OUT {
	vec2 textureCoordinate;
    vec3 normal; // World/Model space
	vec3 tangent; // World/Model space
	vec3 fragPos; // World/Model space
    vec4 fragViewPos; // View space
} fs_in;

layout(binding = 0) uniform sampler2DShadow shadowMap;
layout(binding = 1) uniform sampler2D albedoMap;
layout(binding = 2) uniform sampler2D normalMap;
layout(binding = 3) uniform sampler2D emissionMap;
layout(binding = 4) uniform sampler2D metallicMap;
layout(binding = 5) uniform sampler2D roughnessMap;

layout(binding = 6) uniform sampler2DShadow cascadedShadowMaps1;
layout(binding = 7) uniform sampler2DShadow cascadedShadowMaps2;
layout(binding = 8) uniform sampler2DShadow cascadedShadowMaps3;

const int MAX_POINT_LIGHTS = 16;
layout(binding = 9) uniform samplerCube shadowCubeMapArray[MAX_POINT_LIGHTS];

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

struct PointLight
{
    vec4 position;
    vec4 direction;
    vec4 color;
};

layout(std140, binding = 2) uniform CBLightArray
{
    vec4 lightCount;
    PointLight lights[MAX_POINT_LIGHTS];
} cbLightArray;

layout(std140, binding = 7) uniform CBCascadedShadow
{
    mat4 splits[3];
    vec4 cascadePlaneDistances;
} cbCascadedShadow;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragBrightness;

const float PI = 3.141592;
/*
vec3 ShadowCoords(mat4 shadowMapViewProjection) {
	vec4 fragPosLightSpace = shadowMapViewProjection * vec4(fs_in.fragPos, 1.0);
	// perform perspective divide
    vec3 shadowCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    shadowCoords = shadowCoords * 0.5 + 0.5;
	return shadowCoords;
}
*/

float CascadeDepthSample(int index, vec3 coords) {
    if (index == 0) {
        return texture(cascadedShadowMaps1, coords);
    } else if (index == 1) {
        return texture(cascadedShadowMaps2, coords);
    } else if (index == 2) {
        return texture(cascadedShadowMaps3, coords);
    }
    return 0;
}

float CascadedShadow(vec3 lightDir, vec3 normal) {
    // select cascade layer
    float depthValue = -fs_in.fragViewPos.z;

    const int cascadeCount = 3;
        
    int index = -1;

    for (int i = 0; i < cascadeCount; ++i)
    {
        if (depthValue < cbCascadedShadow.cascadePlaneDistances[i])
        {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return 1;
    }

    vec4 fragPosLightSpace = cbCascadedShadow.splits[index] * vec4(fs_in.fragPos, 1.0);

	// perform perspective divide
    vec3 shadowCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    shadowCoords = shadowCoords * 0.5 + 0.5;

    vec3 projCoords = shadowCoords;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    if (currentDepth  > 1.0) {
        return 1;
    }

    float EPSILON = 0;
    float Factor = 0.0;

    vec2 texelSize = vec2(1.0f);

    if (index == 0) {
        EPSILON = 0.0001;
        texelSize = 1.0f / textureSize(cascadedShadowMaps1, 0);
    } else if (index == 1) {
        EPSILON = 0.0002;
        texelSize = 1.0f / textureSize(cascadedShadowMaps2, 0);
    } else if (index == 2) {
        EPSILON = 0.0001;
        texelSize = 1.0f / textureSize(cascadedShadowMaps3, 0);
    }

    for (int y = -1 ; y <= 1 ; y++) {
        for (int x = -1 ; x <= 1 ; x++) {
            vec2 Offsets = vec2(x * texelSize.x, y * texelSize.y);

            //float pcfDepth = CascadeDepthSample(index, projCoords.xy);
            //Factor += (currentDepth - EPSILON) > pcfDepth ? 1.0 : 0.0;
            
            vec3 UVC = vec3(projCoords.xy + Offsets, currentDepth - EPSILON);
            Factor += CascadeDepthSample(index, UVC);
            
        }
    }
    float shadow = Factor / 9.0f;
    return shadow;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

vec3 CalcDirLightPBR(vec3 L, vec3 N, vec3 V, vec3 albedo) {

    vec3 lightColor = vec3(0.7, 0.65, 0.6);

    float ao = 1.0f;

    float metallic = cbPerFrame.metallic;//0.0;
    float roughness = cbPerFrame.roughness;//0.0;
/*
    if (cbPerObject.hasNormalMap == 1) {
        metallic  = texture(metallicMap, fs_in.textureCoordinate).r;
        roughness = texture(roughnessMap, fs_in.textureCoordinate).r;
    }
*/
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    vec3 H = normalize(V + L);

    vec3 radiance = lightColor * cbPerFrame.sunlightIntensity;

    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);        
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;    

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL; 

    // calculate shadow
    float shadow = 1;

    if (cbPerFrame.sunEnableShadow == 1) {
        shadow = CascadedShadow(L, N);
    }

    // add a fake indirect light...
    float RNdotL = max(dot(N, -L), 0.0);
    vec3 iLo = albedo * radiance * RNdotL * cbPerFrame.sunIndirectIntensity;

    vec3 ambient = vec3(cbPerFrame.sunlightAmbient) * albedo * ao;
    vec3 color = ambient + iLo + (Lo * shadow);
/*
    float depthValue = -fs_in.fragViewPos.z;

    if ( depthValue < cbCascadedShadow.cascadePlaneDistances[0]) {
        color += vec3(0, 0.2, 0);
    } else if ( depthValue < cbCascadedShadow.cascadePlaneDistances[1]) {
        color += vec3(0, 0, 0.2);
    } else if ( depthValue < cbCascadedShadow.cascadePlaneDistances[2]) {
        color += vec3(0.2, 0, 0);
    }
*/
    //color = color / (color + vec3(1.0));
    //color = pow(color, vec3(1.0/2.2));  

    return color;
}

vec3 CalcDirLight(vec3 lightDir, vec3 normal, vec3 viewDir, vec4 color) {

    return CalcDirLightPBR(lightDir, normal, viewDir, color.rgb);
    /*
    vec3 lightColor = vec3(0.5, 0.5, 0.5) * 3;

    // diffuse shading
	float NdotL = clamp(dot(normal, lightDir), 0.0, 1.0);
  	// diffuse
	vec3 diffuse = max(NdotL, 0.0) * lightColor;

    // specular
    vec3 specularColor = vec3(1, 1, 1);

    vec3 halfwayDir = normalize(lightDir + viewDir);

    float NdotH = clamp(dot(normal, halfwayDir), 0.0, 1.0);
    float shininess = 60.0f;
    float spec = pow(NdotH, shininess) * cbPerObject.specularIntensity;
    vec3 specular = (spec * lightColor) * specularColor;
    //vec3 specular = vec3(0, 0, 0);
    // ambient
    float ambientStrength = 0.15;

    vec3 ambient = ambientStrength * lightColor;

    // calculate shadow
    float shadow = CalcDirLightShadow(lightDir, normal);

    vec3 lighting = (ambient + ((diffuse + specular) * shadow)) * color.rgb;

    return lighting;
    */
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 color, int cubeMapIndex) {
    vec3 lightPos = light.position.xyz;
    float farPlane = light.position.w;
    vec3 lightDir = normalize(lightPos - fragPos);

    float lightIntensity = light.color.a;

    vec3 lightColor = light.color.rgb;
    // diffuse shading
    float diff =  max(dot(normal, lightDir), 0.0);

    // specular shading
    /*
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 60) * cbPerObject.specularIntensity;
    */
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float NdotH = clamp(dot(normal, halfwayDir), 0.0, 1.0);
    float shininess = 200.0f;
    float spec = pow(NdotH, shininess) * cbPerObject.specularIntensity;

    float constant = 1.0f;
    float quadratic = 0.0001f;
    float linear = 0.05f;

    // attenuation
    float distance    = length(lightPos - fragPos);
    float attenuation = 1.0 / (constant + linear * distance + 
  			     quadratic * (distance * distance));

    // ambient lighting
	float ambientIntensity = 0.7f;
    vec3 ambientColor = vec3(1, 1, 1);

    // Shadow value
	float shadow = 0.0f;

    if (light.direction.x == 1) {

        vec3 fragToLight = fragPos - lightPos;
        float currentDepth = length(fragToLight);
        float bias = max(0.5f * (1.0f - dot(normal, lightDir)), 0.005f); 

        int sampleRadius = 2;
        float offset = 0.1f;
        for(int z = -sampleRadius; z <= sampleRadius; z++)
        {
            for(int y = -sampleRadius; y <= sampleRadius; y++)
            {
                for(int x = -sampleRadius; x <= sampleRadius; x++)
                {
                    float closestDepth = texture(shadowCubeMapArray[cubeMapIndex], fragToLight + vec3(x, y, z) * offset).r;
                    // Remember that we divided by the farPlane?
                    // Also notice how the currentDepth is not in the range [0, 1]
                    closestDepth *= farPlane;
                    if (currentDepth > closestDepth + bias)
                        shadow += 1.0f;     
                }    
            }
        }
        // Average shadow
        shadow /= pow((sampleRadius * 2 + 1), 3);

    }
    vec3 specularColor = vec3(1, 1, 1);

    vec3 ambient = ambientColor * ambientIntensity;
    vec3 diffuse = (lightColor * diff) * (1.0f - shadow);
    vec3 specular = spec * specularColor;

    //vec3 lighting = ((lightColor * color.rgb) * (diff * attenuation + ambient)) * (1.0f - shadow);

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    vec3 lighting = (ambient + diffuse)  * color.rgb + specular;

    return lighting * lightIntensity;
}

vec4 getBrightness(vec4 color) {
    vec3 luminanceVector = vec3(0.2126, 0.7152, 0.0722);

    float brightness = dot(luminanceVector, color.xyz);

    if (brightness > 1.0) {
        return color;
    } else {
        return vec4(0.0, 0.0, 0.0, 1.0);
    }
}

struct FogParameters
{
	vec3 color;
	float linearStart;
	float linearEnd;
	float density;
	
	int equation;
	bool isEnabled;
};

float getFogFactor(FogParameters params, float fogCoordinate)
{
	float result = 0.0;
	if(params.equation == 0)
	{
		float fogLength = params.linearEnd - params.linearStart;
		result = (params.linearEnd - fogCoordinate) / fogLength;
	}
	else if(params.equation == 1) {
		result = exp(-params.density * fogCoordinate);
	}
	else if(params.equation == 2) {
		result = exp(-pow(params.density * fogCoordinate, 2.0));
	}
	
	result = 1.0 - clamp(result, 0.0, 1.0);
	return result;
}

void main() {

    vec4 color = texture(albedoMap, fs_in.textureCoordinate).rgba;

    if(color.a < 0.8)
        discard;

    vec3 lightDir = normalize(-cbPerFrame.sunDirection.xyz);//normalize(cbPerFrame.lightPosition.xyz - fs_in.fragPos);
	vec3 viewDir = normalize(cbPerFrame.cameraPosition.xyz - fs_in.fragPos);

    vec3 normal;

    if (cbPerObject.hasNormalMap == 1) {
        vec3 N = normalize(fs_in.normal);
        vec3 T = normalize(fs_in.tangent);
        // re-orthogonalize T with respect to N
        T = normalize(T - dot(T, N) * N);
        // then retrieve perpendicular vector B with the cross product of T and N
        vec3 B = normalize(cross(N, T));

        mat3 TBN = inverse(mat3(T, B, N));

        // tangent-space normal 
        vec3 bumpNormal = normalize(texture(normalMap, fs_in.textureCoordinate).xyz * 2.0f - 1.0f);
        normal = normalize(bumpNormal * TBN);
    } else {
        normal = normalize(fs_in.normal);
    }

    vec3 lighting = vec3(0, 0, 0);

    lighting += CalcDirLight(lightDir, normal, viewDir, color);

    int cubeShadowMapIndex = 0;
    for(int i = 0; i < cbLightArray.lightCount.x; i++) {
        PointLight light = cbLightArray.lights[i];
        lighting += CalcPointLight(light, normal, fs_in.fragPos, viewDir, color, cubeShadowMapIndex);
        if (light.direction.x == 1) {
            cubeShadowMapIndex++;
        }
    }

    if (cbPerObject.hasEmissionMap == 1) {
        lighting += texture(emissionMap, fs_in.textureCoordinate).rgb * 5;
    }

    vec4 finalColor = vec4(lighting, color.a);

    
    //if (!gl_FrontFacing) {
        //lighting = vec3(1, 0, 0);
    //}

    FogParameters fogParams;

    fogParams.equation = 2;
    fogParams.color = vec3(0.3, 0.3, 0.3);
    fogParams.linearStart = 200;
    fogParams.linearEnd = 5000;
    fogParams.density = cbPerFrame.fogDensity;//0.001;

    float fogCoordinate = abs(fs_in.fragViewPos.z / fs_in.fragViewPos.w);
    finalColor = mix(finalColor, vec4(fogParams.color, 1.0), getFogFactor(fogParams, fogCoordinate));

    fragColor = finalColor;

    fragBrightness = getBrightness(fragColor);
}


