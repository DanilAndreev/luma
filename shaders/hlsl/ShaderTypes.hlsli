#ifndef __LUMA_HLSL_SHADER_TYPES__
#define __LUMA_HLSL_SHADER_TYPES__

#include "HLSLCPPCompat.hlsli"

#define LUMA_DIR_SHADOW_MAP_DIM 4096;
#define LUMA_OMNIDIR_SHADOW_MAP_DIM 2048;

namespace HLSL {

    struct CameraParams {
        float4x4 worldToCamera;
        float4x4 cameraToProjection;
        float4x4 worldToCameraProj;
        float3 worldPos;
        float padding;
    };

    struct MaterialParams {
        float3 color;
        float shininess;

        uint hasDiffuseMap;
        uint hasSpecularMap;
        uint hasNormalMap;
        uint hasHeightMap;
    };

    struct MeshParams {
        float4x4 modelToWorld;
        float4x4 modelToWorldNormal;
        MaterialParams material;
    };

    struct DirectionalLight {
        float4x4 worldToLightProj;
        float4x4 worldToLight;
        float4x4 lightToProj;

        float3 color;
        float ambientIntensity;

        float3 direction;
        float intensity;
    };

    struct PointLight {
        float3 position;
        float ambientIntensity;

        float3 color;
        float padding1;

        float constantAttenuation;
        float linearAttenuation;
        float quadraticAttenuation;
        uint shadowMapResolutionDim;

        float4x4 shadowMapProjInv;

        float shadowMapProjFarPlane;
        float shadowMapProjNearPlane;
        float2 padding2;
    };

    struct LightParams {
        DirectionalLight dirLight;

        uint pointLightCount;
        float3 padding;
    };
} /* namespace HLSL*/

#endif // __LUMA_HLSL_SHADER_TYPES__