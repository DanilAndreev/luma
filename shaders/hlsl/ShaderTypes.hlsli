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
        float4 ambientColor;
        float4 diffuseColor;
        float4 specularColor;
        float4 position;

        float constantAttenuation;
        float linearAttenuation;
        float quadraticAttenuation;
        float padding;
    };

} /* namespace HLSL*/

#endif // __LUMA_HLSL_SHADER_TYPES__