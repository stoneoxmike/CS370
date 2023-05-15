#version 400 core
uniform sampler2D shadowMap;

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

// Material structure
struct MaterialProperties {
     vec4 ambient;
     vec4 diffuse;
     vec4 specular;
     float shininess;
};

const int MaxLights = 8;
layout (std140) uniform LightBuffer {
     LightProperties Lights[MaxLights];
};

const int MaxMaterials = 8;
layout (std140) uniform MaterialBuffer {
     MaterialProperties Materials[MaxMaterials];
};

// Selected material
uniform int Material;

// Number of lights
uniform int NumLights;
uniform int LightOn[MaxLights];

out vec4 fragColor;

in vec4 Position;
in vec3 Normal;
in vec3 View;
in vec4 LightPosition;

// TODO: Perform shadow depth comparison
float ShadowCalculation(vec4 fragLightPos) {
    // Normalize light position [-1, 1]
    vec3 projCoords = fragLightPos.xyz/fragLightPos.w;

    // Convert to depth range [0, 1]
    projCoords = projCoords*0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float curDepth = projCoords.z;

    float bias = 0.005;
    return curDepth - bias > closestDepth ? 1.0f : 0.0f;
}

void main()
{
     vec3 rgb = vec3(0.0f);
     vec3 NormNormal = normalize(Normal);
     vec3 NormView = normalize(View);

     for (int i = 0; i < NumLights; i++) {
          // If light is not off
          if (LightOn[i] != 0) {
               // Ambient component
               if (Lights[i].type != 0) {
                    rgb += vec3(Lights[i].ambient*Materials[Material].ambient);
               }
               // Directional Light
               if (Lights[i].type == 1) {
                    vec3 LightDirection = -normalize(vec3(Lights[i].direction));
                    vec3 HalfVector = normalize(LightDirection + NormView);
                    // Diffuse
                    float diff = max(0.0f, dot(NormNormal, LightDirection));
                    rgb += diff*vec3(Lights[i].diffuse*Materials[Material].diffuse);
                    if (diff > 0.0) {
                         // Specular term
                         float spec = pow(max(0.0f, dot(Normal, HalfVector)), Materials[Material].shininess);
                         rgb += spec*vec3(Lights[i].specular*Materials[Material].specular);
                    }
               }
               // Point light
               if (Lights[i].type == 2) {
                    vec3 LightDirection = normalize(vec3(Lights[i].position - Position));
                    vec3 HalfVector = normalize(LightDirection + NormView);
                    // Diffuse
                    float diff = max(0.0f, dot(NormNormal, LightDirection));
                    rgb += diff*vec3(Lights[i].diffuse*Materials[Material].diffuse);
                    if (diff > 0.0) {
                         // Specular term
                         float spec = pow(max(0.0f, dot(Normal, HalfVector)), Materials[Material].shininess);
                         rgb += spec*vec3(Lights[i].specular*Materials[Material].specular);
                    }
               }
               // Spot light
               if (Lights[i].type == 3) {
                    vec3 LightDirection = normalize(vec3(Lights[i].position - Position));
                    // Determine if inside cone
                    float spotCos = dot(LightDirection, -normalize(vec3(Lights[i].direction)));
                    float coneCos = cos(radians(Lights[i].spotCutoff));
                    if (spotCos >= coneCos) {
                         vec3 HalfVector = normalize(LightDirection + NormView);
                         float attenuation = pow(spotCos, Lights[i].spotExponent);
                         // Diffuse
                         float diff = max(0.0f, dot(NormNormal, LightDirection))*attenuation;
                         rgb += diff*vec3(Lights[i].diffuse*Materials[Material].diffuse);
                         if (diff > 0.0) {
                              // Specular term
                              float spec = pow(max(0.0f, dot(Normal, HalfVector)), Materials[Material].shininess)*attenuation;
                              rgb += spec*vec3(Lights[i].specular*Materials[Material].specular);
                         }
                    }
               }
          }
     }

     // TODO: Determine if fragment (LightPosition) is in shadow
     float shadow = 1.0 - ShadowCalculation(LightPosition);
     // TODO: Apply shadow attenuation to base color
     fragColor = shadow*vec4(min(rgb,vec3(1.0)), 1.0f);
}
