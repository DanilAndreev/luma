#include "vertexSemantics.hlsli"

#include "ShaderTypes.hlsli"

cbuffer CB_CameraParams : register(b0) { HLSL::CameraParams CBCameraParams; }
cbuffer CB_MeshParams : register(b1) { HLSL::MeshParams CBMeshParams; }

VSOut VSMain(VSIn input) {
    VSOut output;

    float4x4 MVP = mul(CBMeshParams.transform, CBCameraParams.worldToCameraProj);
    output.position = mul(input.position, MVP);
    //TODO: calculate normal matrix and transform normals.
    output.normal = input.normal;
    //output.texcoor0 = input.texcoor0;
    //output.color0 = input.color0;
    output.worldPos = mul(input.position, CBMeshParams.transform).xyz;
    return output;
}
