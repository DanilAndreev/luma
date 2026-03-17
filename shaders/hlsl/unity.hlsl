#include "vertexSemantics.hlsli"
#include "ShaderTypes.hlsli"

cbuffer CB_CameraParams : register(b0) { HLSL::CameraParams CBCameraParams; }
cbuffer CB_MeshParams : register(b1) { HLSL::MeshParams CBMeshParams; }
cbuffer CB_LightParams : register(b2) { HLSL::LightParams CBLightParams; }

StructuredBuffer<HLSL::PointLight> SRVPointLight : register(t0);

Texture2D<float> DirLightShadowMap : register(t1);
TextureCubeArray<float> PointLightShadowMap : register(t2);

Texture2D<float4> SRVDiffuseColorMap : register(t3);
Texture2D<float> SRVSpecularMap : register(t4);
Texture2D<float3> SRVNormalMap : register(t5);
Texture2D<float> SRVHeightMap : register(t6);

SamplerState ShadowMapSMP : register(s0);
SamplerState LightMapSMP : register(s1);

struct PSOut
{
    float4 color : SV_Target0;
    float4 debugColor : SV_Target1;
};

float PointLightShadowCalculation(HLSL::PointLight light, uint lightIdx, float3 worldPos) {
    float3 fragToLight = worldPos - light.position;
    float closestDepth = PointLightShadowMap.Sample(ShadowMapSMP, float4(fragToLight, lightIdx));
    float closestDepthLinear = closestDepth * light.shadowMapProjFarPlane;
    float currentDepth = length(fragToLight);
    float bias = 0.05;
    return currentDepth - bias > closestDepthLinear ? 1.0 : 0.0;
}

float3 PointLight(VSOut input, HLSL::PointLight light, uint lightIdx, float3 viewDir, float3 objectColor, float3 normal) {
    float3 lightDir = normalize(light.position.xyz - input.worldPos);
    float diffuseStrength = max(dot(normal, lightDir), 0.0);

    float3 halfwayDir = normalize(lightDir + viewDir);
    float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), CBMeshParams.material.shininess);
    float lightDistance = length(light.position.xyz - input.worldPos);
    float attenuation = 1.0 / (light.constantAttenuation + light.linearAttenuation * lightDistance + light.quadraticAttenuation * (lightDistance * lightDistance));

    float3 ambient  = light.color * objectColor * light.ambientIntensity * light.intensity;
    float3 diffuse  = light.color * objectColor * diffuseStrength * light.intensity;
    float3 specular = light.color * objectColor * specularStrength * light.intensity;

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

float3 DirectionalLight(VSOut input, HLSL::DirectionalLight light, float3 viewDir, float3 objectColor, float3 normal) {
    float3 lightDir = normalize(-light.direction);
    float diffuseStrength = max(dot(normal, lightDir), 0.0);

    float3 halfwayDir = normalize(lightDir + viewDir);
    float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), CBMeshParams.material.shininess);

    float3 ambient  = light.color * objectColor * light.ambientIntensity;
    float3 diffuse  = light.color * objectColor * diffuseStrength;
    float3 specular = light.color * objectColor * specularStrength;

    float4 lightSpacePos = mul(float4(input.worldPos, 1.0f), light.worldToLightProj);
    float shadow = DirlightShadowCalculation(lightSpacePos);
    return (ambient + (1.0f - shadow) * (diffuse + specular)) * light.intensity;
}

PSOut PSMain(VSOut input) {
    float3 viewDir = normalize(CBCameraParams.worldPos - input.worldPos);

	float3x3 TBN = float3x3(input.tangent, input.bitangent, input.normal.xyz);

	float3 normal;
	if (CBMeshParams.material.hasNormalMap) {
	    float3 sampledNormal = SRVNormalMap.Sample(LightMapSMP, input.texcoord0) * 2.0f - 1.0f;
	    //TODO: inverted for texture
	    sampledNormal.y *= -1;
		normal = mul(sampledNormal, TBN);
	} else {
	    normal = normalize(input.normal.xyz);
	}

	float3 objectColor;
	if (CBMeshParams.material.hasDiffuseMap) {
		objectColor = SRVDiffuseColorMap.Sample(LightMapSMP, input.texcoord0).rgb;
	} else {
		objectColor = CBMeshParams.material.color;
	}

    PSOut output;
    output.color = 0.0f;
    for (uint i = 0; i < CBLightParams.pointLightCount; ++i) {
        output.color += float4(PointLight(input, SRVPointLight[i], i, viewDir, objectColor, normal), 0.0f);
    }
    output.color += float4(DirectionalLight(input, CBLightParams.dirLight, viewDir, objectColor, normal), 0.0f);

    output.debugColor = 0.0f;
    return output;
}
