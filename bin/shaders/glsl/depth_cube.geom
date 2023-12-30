#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout(std140, binding = 3) uniform CBShadowCube
{
    mat4 shadowMatrices[6];
    vec4 lightPos;
} cbShadowCube;

out vec4 FragPos;

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        // Sets the face we are currently working on
        gl_Layer = face;
        for(int i = 0; i < 3; i++)
        {
            // Make transformed vertex
            FragPos = gl_in[i].gl_Position;
            gl_Position = cbShadowCube.shadowMatrices[face] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}

