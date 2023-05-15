#version 400 core
uniform sampler2D baseMap;
uniform sampler2D normalMap;

// Light structure
struct LightProperties {
    int type;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 position;
    vec4 direction;
    float spotCutoff;
    float spotExponent;
};

const int MaxLights = 8;
layout (std140) uniform LightBuffer {
    LightProperties Lights[MaxLights];
};

uniform int NumLights;
uniform int LightOn[MaxLights];

out vec4 fragColor;

in vec4 Position;
in vec3 Normal;
in vec3 Tangent;
in vec3 BiTangent;
in vec2 texCoord;
in vec3 View;

void main()
{
    vec3 rgb = vec3(0.0f);
    vec3 NormNormal = normalize(Normal);
    vec3 NormView = normalize(View);

    // Retrieve normal from normal map
    vec4 BumpCol = texture(normalMap, texCoord);
    // TODO: Compute perturbed per pixel normal vector from normal map color
    vec3 BumpNorm = normalize(2.0f*BumpCol.rgb - 1.0f);

    // TODO: Convert view vector to tangent space
    vec3 TangView = normalize(vec3(dot(Tangent, NormView),dot(BiTangent, NormView),dot(Normal, NormView)));

    for (int i = 0; i < NumLights; i++) {

        // If light is not off
        if (LightOn[i] != 0) {
            // add ambient component
            if (Lights[i].type != 0) {
                // Ambient
                rgb += vec3(Lights[i].ambient);
            }
            // Directional Light
            if (Lights[i].type == 1) {
                vec3 LightDir = -normalize(vec3(Lights[i].direction));
                // TODO: Compute light vector to tangent space
                vec3 LightDirection = vec3(dot(Tangent, LightDir), dot(BiTangent, LightDir), dot(Normal, LightDir));

                LightDirection = normalize(LightDirection);
                vec3 HalfVector = normalize(LightDirection + TangView);
                // Diffuse
                float diff = max(0.0f, dot(BumpNorm, LightDirection));
                rgb += diff*vec3(Lights[i].diffuse);
                if (diff > 0.0) {
                    float spec = max(0.0f, dot(BumpNorm, HalfVector));
                    rgb += spec*vec3(Lights[i].specular);
                }
            }
            // Point light
            if (Lights[i].type == 2) {
                vec3 LightDir = normalize(vec3(Lights[i].position - Position));
                // TODO: Compute light vector to tangent space
                vec3 LightDirection = vec3(dot(Tangent, LightDir), dot(BiTangent, LightDir), dot(Normal, LightDir));

                LightDirection = normalize(LightDirection);
                vec3 HalfVector = normalize(LightDirection + TangView);
                // Diffuse
                float diff = max(0.0f, dot(BumpNorm, LightDirection));
                rgb += diff*vec3(Lights[i].diffuse);
                if (diff > 0.0) {
                    float spec = max(0.0f, dot(BumpNorm, HalfVector));
                    rgb += spec*vec3(Lights[i].specular);
                }
            }
            // Spot light
            if (Lights[i].type == 3) {
                vec3 LightDir = normalize(vec3(Lights[i].position - Position));
                // TODO: Compute light vector to tangent space
                vec3 LightDirection = vec3(dot(Tangent, LightDir), dot(BiTangent, LightDir), dot(Normal, LightDir));

                LightDirection = normalize(LightDirection);
                // Compute amount inside cone
                float spotCos = dot(LightDir, -normalize(vec3(Lights[i].direction)));
                float coneCos = cos(radians(Lights[i].spotCutoff));
                if (spotCos >= coneCos) {
                    vec3 HalfVector = normalize(LightDirection + TangView);
                    float attenuation = pow(spotCos, Lights[i].spotExponent);
                    // Diffuse
                    float diff = max(0.0f, dot(BumpNorm, LightDirection))*attenuation;
                    rgb += diff*vec3(Lights[i].diffuse);
                    if (diff > 0.0) {
                        // Specular term
                        float spec = max(0.0f, dot(Normal, HalfVector))*attenuation;
                        rgb += spec*vec3(Lights[i].specular);
                    }
                }
            }
        }
    }

    // TODO: Multiply the lighting effect by the base texture color
    fragColor = vec4(rgb,1.0)*texture(baseMap, texCoord);
}

