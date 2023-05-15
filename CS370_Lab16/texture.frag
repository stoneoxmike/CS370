#version 400 core
uniform sampler2D tex;

out vec4 fragColor;

in vec2 texCoord;

void main()
{
    // Sample texture map
    fragColor = texture(tex, texCoord);
}
