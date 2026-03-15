#include "vertexSemantics.hlsli"
#include "ShaderTypes.hlsli"

cbuffer CB_CameraParams : register(b0) { HLSL::CameraParams CBCameraParams; }
cbuffer CB_MeshParams : register(b1) { HLSL::MeshParams CBMeshParams; }
cbuffer CB_LightParams : register(b2) { HLSL::LightParams CBLightParams; }

StructuredBuffer<HLSL::PointLight> SRVPointLight : register(t0);

Texture2D<float> DirLightShadowMap : register(t1);

SamplerState ShadowMapSMP : register(s0);

struct PSOut
{
    float4 color : SV_Target0;
    float4 debugColor : SV_Target1;
};

float3 PointLight(VSOut input, HLSL::PointLight light, float3 viewDir, float3 objectColor) {
    float3 normal = normalize(input.normal.xyz);
    float3 lightDir = normalize(light.position.xyz - input.worldPos);
    float diffuseStrength = max(dot(normal, lightDir), 0.0);

    float3 halfwayDir = normalize(lightDir + viewDir);
    float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), CBMeshParams.material.shininess);
    float lightDistance = length(light.position.xyz - input.worldPos);
    float attenuation = 1.0 / (light.constantAttenuation + light.linearAttenuation * lightDistance + light.quadraticAttenuation * (lightDistance * lightDistance));

    float3 ambient  = light.ambientColor * objectColor;
    float3 diffuse  = light.diffuseColor * diffuseStrength * objectColor;
    float3 specular = light.specularColor * specularStrength * objectColor;
    return diffuse * attenuation + ambient * attenuation + specular * attenuation;
}

float ShadowCalculation(float4 posLightSpace)
{
    float3 projCoords = posLightSpace.xyz / posLightSpace.w;
    projCoords.x = projCoords.x * 0.5f + 0.5f;
    projCoords.y = projCoords.y * 0.5f + 0.5f;
    projCoords.y = 1.0f - projCoords.y;

    const float bias = 0.005;
    float closestDepth = DirLightShadowMap.Sample(ShadowMapSMP, projCoords.xy).r;
    return projCoords.z - bias > closestDepth ? 1.0 : 0.0;
}

float3 DirectionalLight(VSOut input, HLSL::DirectionalLight light, float3 viewDir, float3 objectColor) {

    float3 normal = normalize(input.normal.xyz);
    float3 lightDir = normalize(-light.direction);
    float diffuseStrength = max(dot(normal, lightDir), 0.0);

    float3 halfwayDir = normalize(lightDir + viewDir);
    float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), CBMeshParams.material.shininess);

    float3 ambient  = light.ambientColor * objectColor;
    float3 diffuse  = light.diffuseColor * diffuseStrength * objectColor;
    float3 specular = light.specularColor * specularStrength * objectColor;

    float4 lightSpacePos = mul(float4(input.worldPos, 1.0f), light.worldToLightProj);
    float shadow = ShadowCalculation(lightSpacePos);
    return (ambient + (1.0f - shadow) * (diffuse + specular)) * light.intensity;
}

PSOut PSMain(VSOut input) {
    float3 viewDir = normalize(CBCameraParams.worldPos - input.worldPos);

    float4 objectColor = float4(1.0f, 0.5f, 0.0f, 1.0f); // TODO: load color from texmap

    PSOut output;
    output.color = 0.0f;
    for (uint i = 0; i < CBLightParams.pointLightCount; ++i) {
        output.color += float4(PointLight(input, SRVPointLight[i], viewDir, objectColor.xyz), 0.0f);
    }
    output.color += float4(DirectionalLight(input, CBLightParams.dirLight, viewDir, objectColor.xyz), 0.0f);

    output.debugColor = 0.0f;
    return output;
}
