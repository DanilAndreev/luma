#ifndef __LUMA_HLSL_SHADER_TYPES__
#define __LUMA_HLSL_SHADER_TYPES__

#include "HLSLCPPCompat.hlsli"

namespace HLSL {

    struct CameraParams {
        float4x4 worldToCamera;
        float4x4 cameraToProjection;
    };

    struct MaterialParams {
        float4 ambientColor;

        float ambientStrength;
        uint pointLightCount;
        float2 padding;
    };

    struct PointLight {
        float4 position;
        float4 color;
        float shininess;
        float3 padding;
    };

} /* namespace HLSL*/

#endif // __LUMA_HLSL_SHADER_TYPES__