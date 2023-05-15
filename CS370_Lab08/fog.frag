#version 400 core
// TODO: Define input shader variable (vec4)
in vec4 oColor;
// TODO: Define output shader variable (vec4)
out vec4 fragColor;
// TODO: Define uniform shader variables
uniform vec4 fog_color;
uniform float fog_start;
uniform float fog_end;
void main()
{
    // TODO: Compute fog factor
    float fog = (fog_end - (gl_FragCoord.z/gl_FragCoord.w))/(fog_end - fog_start);
    fog = clamp(fog, 0.0, 1.0);
    fragColor = mix(fog_color, oColor, fog);
    // TODO: Compute output color
    //fragColor = oColor;
}
