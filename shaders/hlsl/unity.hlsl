#include "vertexSemantics.hlsli"
#include "ShaderTypes.hlsli"

cbuffer CB_CameraParams : register(b0) { HLSL::CameraParams CBCameraParams; }
cbuffer CB_MeshParams : register(b1) { HLSL::MeshParams CBMeshParams; }
cbuffer CB_LightParams : register(b2) { HLSL::LightParams CBLightParams; }

StructuredBuffer<HLSL::PointLight> SRVPointLight : register(t0);

Texture2D<float> DirLightShadowMap : register(t1);
TextureCubeArray<float> PointLightShadowMap : register(t2);

SamplerState ShadowMapSMP : register(s0);

struct PSOut
{
    float4 color : SV_Target0;
    float4 debugColor : SV_Target1;
};

struct ObjectColor {
    float3 ambientColor;
    float3 diffuseColor;
    float3 specularColor;
};

float PointLightShadowCalculation(HLSL::PointLight light, uint lightIdx, float3 worldPos) {
    float3 fragToLight = worldPos - light.position;
    float closestDepth = PointLightShadowMap.Sample(ShadowMapSMP, float4(fragToLight, lightIdx));
    float closestDepthLinear = closestDepth * light.shadowMapProjFarPlane;
    float currentDepth = length(fragToLight);
    float bias = 0.05;
    return currentDepth - bias > closestDepthLinear ? 1.0 : 0.0;
}

float3 PointLight(VSOut input, HLSL::PointLight light, uint lightIdx, float3 viewDir, ObjectColor objectColor) {
    float3 normal = normalize(input.normal.xyz);
    float3 lightDir = normalize(light.position.xyz - input.worldPos);
    float diffuseStrength = max(dot(normal, lightDir), 0.0);

    float3 halfwayDir = normalize(lightDir + viewDir);
    float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), CBMeshParams.material.shininess);
    float lightDistance = length(light.position.xyz - input.worldPos);
    float attenuation = 1.0 / (light.constantAttenuation + light.linearAttenuation * lightDistance + light.quadraticAttenuation * (lightDistance * lightDistance));

    float3 ambient  = light.ambientColor * objectColor.ambientColor;
    float3 diffuse  = light.diffuseColor * diffuseStrength * objectColor.diffuseColor;
    float3 specular = light.specularColor * specularStrength * objectColor.specularColor;

    float shadow = PointLightShadowCalculation(light, lightIdx, input.worldPos);
    return ambient * attenuation + (1 - shadow) * (diffuse * attenuation + specular * attenuation);
}

// float LinearizeDepth(float depth)
// {
//     float z = depth * 2.0 - 1.0; // Back to NDC
//     return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
// }

float DirlightShadowCalculation(float4 posLightSpace)
{
    float3 projCoords = posLightSpace.xyz / posLightSpace.w;
    projCoords.x = projCoords.x * 0.5f + 0.5f;
    projCoords.y = projCoords.y * 0.5f + 0.5f;
    projCoords.y = 1.0f - projCoords.y;

    const float bias = 0.005;

    float shadow = 0.0;
    float2 texelSize = 1.0 / LUMA_DIR_SHADOW_MAP_DIM;

    [unroll]
    for(int x = -1; x <= 1; ++x)
    {
        [unroll]
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = DirLightShadowMap.Sample(ShadowMapSMP, projCoords.xy + float2(x, y) * texelSize);
            shadow += projCoords.z - bias > pcfDepth ? 1.0f : 0.0f;
        }
    }
    return shadow / 9.0f;
}

float3 DirectionalLight(VSOut input, HLSL::DirectionalLight light, float3 viewDir, ObjectColor objectColor) {

    float3 normal = normalize(input.normal.xyz);
    float3 lightDir = normalize(-light.direction);
    float diffuseStrength = max(dot(normal, lightDir), 0.0);

    float3 halfwayDir = normalize(lightDir + viewDir);
    float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), CBMeshParams.material.shininess);

    float3 ambient  = light.ambientColor * objectColor.ambientColor;
    float3 diffuse  = light.diffuseColor * diffuseStrength * objectColor.diffuseColor;
    float3 specular = light.specularColor * specularStrength * objectColor.specularColor;

    float4 lightSpacePos = mul(float4(input.worldPos, 1.0f), light.worldToLightProj);
    float shadow = DirlightShadowCalculation(lightSpacePos);
    return (ambient + (1.0f - shadow) * (diffuse + specular)) * light.intensity;
}

PSOut PSMain(VSOut input) {
    float3 viewDir = normalize(CBCameraParams.worldPos - input.worldPos);

	// TODO: load color from texmap if present
	ObjectColor objectColor;
	objectColor.ambientColor = CBMeshParams.material.ambientColor;
	objectColor.diffuseColor = CBMeshParams.material.diffuseColor;
	objectColor.specularColor = CBMeshParams.material.specularColor;

    PSOut output;
    output.color = 0.0f;
    for (uint i = 0; i < CBLightParams.pointLightCount; ++i) {
        output.color += float4(PointLight(input, SRVPointLight[i], i, viewDir, objectColor), 0.0f);
    }
    output.color += float4(DirectionalLight(input, CBLightParams.dirLight, viewDir, objectColor), 0.0f);

    output.debugColor = 0.0f;
    return output;
}
