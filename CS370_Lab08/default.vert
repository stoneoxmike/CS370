#version 400 core
// TODO: Add uniform matrices (mat4)
uniform mat4 proj_matrix;
uniform mat4 camera_matrix;
uniform mat4 model_matrix;
// TODO: Add vertex attribute variables
layout(location=0) in vec4 vPosition;
layout(location=1) in vec4 vColor;
// TODO: Add output variable (vec4)
out vec4 oColor;
void main()
{
    // TODO: Compute gl_Position transformed position
    gl_Position = proj_matrix*camera_matrix*model_matrix*vPosition;
    // TODO: Set oColor output
    oColor = vColor;
}
