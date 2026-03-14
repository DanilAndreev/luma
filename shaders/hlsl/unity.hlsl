#include "vertexSemantics.hlsli"
#include "ShaderTypes.hlsli"

cbuffer CB_CameraParams : register(b0) { HLSL::CameraParams CBCameraParams; }
cbuffer CB_MaterialParams : register(b1) { HLSL::MaterialParams CBMaterialParams; }
StructuredBuffer<HLSL::PointLight> SRVPointLight : register(t0);

struct PSOut
{
    float4 color : SV_Target;
};

float4 PointLight(VSOut input, HLSL::PointLight light, float3 viewDir, float4 objectColor) {
    float3 normal = normalize(input.normal.xyz);
    float3 lightDir = normalize(light.position.xyz - input.worldPos);
    float diffuseStrength = max(dot(normal, lightDir), 0.0);

    float3 reflectDir = reflect(-lightDir, normal);
    float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0), CBMaterialParams.shininess);
    float lightDistance = length(light.position.xyz - input.worldPos);
    float attenuation = 1.0 / (light.constantAttenuation + light.linearAttenuation * lightDistance + light.quadraticAttenuation * (lightDistance * lightDistance));

    float4 ambient  = light.ambientColor * objectColor;
    float4 diffuse  = light.diffuseColor * diffuseStrength * objectColor;
    float4 specular = light.specularColor * specularStrength * objectColor;
    return diffuse * attenuation + ambient * attenuation + specular * attenuation;
}

PSOut PSMain(VSOut input) {
    float3 viewDir = normalize(CBCameraParams.worldPos - input.worldPos);

    float4 objectColor = float4(1.0f, 0.5f, 0.0f, 1.0f); // TODO: load color from texmap

    PSOut output;
    output.color = 0.0f;
    for (uint i = 0; i < CBMaterialParams.pointLightCount; ++i) {
        output.color = PointLight(input, SRVPointLight[i], viewDir, objectColor);
    }
    return output;
}
