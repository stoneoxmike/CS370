#version 400 core
uniform sampler2D baseMap;
uniform sampler2D dirtMap;

out vec4 fragColor;

in vec2 texCoord;

uniform float mixFactor;

void main()
{
    // Sample texture map
    vec4 baseColor = texture(baseMap, texCoord);

    // TODO: Sample dirt texture
    vec4 dirtColor = texture(dirtMap, texCoord);
    // TODO: Mix texture colors
    fragColor = mix(baseColor,dirtColor,mixFactor);
}
