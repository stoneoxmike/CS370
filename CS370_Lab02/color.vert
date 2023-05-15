#version 400 core
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec4 vColor;

out vec4 oColor;

void main()
{
    gl_Position = vPosition;
    oColor = vColor;
}
