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
        uint pointLightCount;

        float2 padding;
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

} /* namespace HLSL*/

#endif // __LUMA_HLSL_SHADER_TYPES__