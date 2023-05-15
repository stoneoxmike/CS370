#version 400 core
uniform mat4 proj_matrix;
uniform mat4 camera_matrix;
uniform mat4 normal_matrix;
uniform mat4 model_matrix;

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;
layout(location = 4) in vec3 vBiTangent;

uniform vec3 EyePosition;

out vec4 Position;
out vec2 texCoord;
out vec3 Normal;
out vec3 Tangent;
out vec3 BiTangent;
out vec3 View;

void main( )
{
    // Compute transformed vertex position in view space
    gl_Position = proj_matrix*(camera_matrix*(model_matrix*vPosition));

    // Compute transformed vertex in world space
    Position = model_matrix*vPosition;

    // Compute v (camera location - transformed vertex) (passed to fragment shader)
    View = normalize(EyePosition - Position.xyz);

    // Pass texture coordinate to frag shader
    texCoord = vTexCoord;

    // Compute tangent space vectors
    Normal = vec3(normalize(normal_matrix*normalize(vec4(vNormal, 0.0))));
    Tangent = vec3(normalize(normal_matrix*normalize(vec4(vTangent, 0.0))));
    BiTangent = cross(Normal, Tangent);
}
