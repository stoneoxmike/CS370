#version 400 core
out vec4 fragColor;

in vec3 oColor;

void main()
{
     fragColor = vec4(min(oColor,vec3(1.0)), 1.0f);
}
