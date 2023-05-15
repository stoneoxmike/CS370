#version 330 core
uniform mat4 light_proj_matrix;
uniform mat4 light_cam_matrix;
uniform mat4 model_matrix;

layout(location = 0) in vec4 vPosition;

void main( )
{
    // Compute transformed vertex position in light space
    gl_Position = light_proj_matrix*(light_cam_matrix*(model_matrix*vPosition));

}
