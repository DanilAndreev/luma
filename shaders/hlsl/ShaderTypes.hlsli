#ifndef __LUMA_HLSL_SHADER_TYPES__
#define __LUMA_HLSL_SHADER_TYPES__

#include "HLSLCPPCompat.hlsli"

namespace HLSL {

    struct CameraParams {
        float4x4 worldToCamera;
        float4x4 cameraToProjection;
    };

} /* namespace HLSL*/

#endif // __LUMA_HLSL_SHADER_TYPES__