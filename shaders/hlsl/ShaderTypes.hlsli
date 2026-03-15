#ifndef __LUMA_HLSL_SHADER_TYPES__
#define __LUMA_HLSL_SHADER_TYPES__

#include "HLSLCPPCompat.hlsli"

namespace HLSL {

    struct CameraParams {
        float4x4 worldToCamera;
        float4x4 cameraToProjection;
        float3 worldPos;
        float padding;
    };

    struct MaterialParams {
        float shininess;

        float3 padding;
    };

    struct MeshParams {
        float4x4 transform;
        MaterialParams material;
    };

    struct DirectionalLight {
        float4x4 worldToLightProj;
        float4x4 worldToLight;
        float4x4 lightToProj;
        float3 ambientColor;
        float padding1;
        float3 diffuseColor;
        float padding2;
        float3 specularColor;
        float padding3;
        float3 direction;
        float intensity;
    };

    struct PointLight {
        float3 ambientColor;
        float constantAttenuation;
        float3 diffuseColor;
        float linearAttenuation;
        float3 specularColor;
        float quadraticAttenuation;
        float4 position;
    };

    struct LightParams {
        DirectionalLight dirLight;

        uint pointLightCount;
        float3 padding;
    };
} /* namespace HLSL*/

#endif // __LUMA_HLSL_SHADER_TYPES__