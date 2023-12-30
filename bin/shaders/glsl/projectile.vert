

layout (location = 0) in vec3 position;
layout (location = 3) in vec2 textureCoord;

layout(std140, binding = 1) uniform CBPerObject
{
    mat4 world;
    float hasNormalMap;
    float hasEmissionMap;
    float opacity;
    float specularIntensity;
    float specularGlossiness;
    float emissionIntensity;
    float pad1;
    float pad2;
} cbPerObject;

out vec2 textureCoordinate;

void main(){
    mat4 modelView = cbPerFrame.view * cbPerObject.world;
/*
    int spherical = 1; // 1 for spherical; 0 for cylindrical

    // First colunm.
    modelView[0][0] = 1.0; 
    modelView[0][1] = 0.0; 
    modelView[0][2] = 0.0; 

    if (spherical == 1)
    {
        // Second colunm.
        modelView[1][0] = 0.0; 
        modelView[1][1] = 1.0; 
        modelView[1][2] = 0.0; 
    }

    // Thrid colunm.
    modelView[2][0] = 0.0; 
    modelView[2][1] = 0.0; 
    modelView[2][2] = 1.0; 
*/
	gl_Position = cbPerFrame.proj * modelView * vec4(position, 1.0);
	textureCoordinate = textureCoord ;
}
