#include "vertexSemantics.hlsli"

#include "ShaderTypes.hlsli"

cbuffer CB_CameraParams : register(b0) { HLSL::CameraParams CBCameraParams; }

VSOut VSMain(VSIn input) {
    VSOut output;

    float4x4 viewProj = mul(CBCameraParams.worldToCamera, CBCameraParams.cameraToProjection);
    output.position = mul(input.position, viewProj);
    output.normal = input.normal;
    //output.texcoor0 = input.texcoor0;
    //output.color0 = input.color0;
    output.worldPos = input.position.xyz;
    return output;
}
