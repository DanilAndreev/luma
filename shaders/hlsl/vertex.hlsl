#include "vertexSemantics.hlsli"

#include "ShaderTypes.hlsli"

cbuffer CB_CameraParams : register(b0) { HLSL::CameraParams CBCameraParams; }
cbuffer CB_MeshParams : register(b1) { HLSL::MeshParams CBMeshParams; }

VSOut VSMain(VSIn input) {
    VSOut output;

    float4x4 MVP = mul(CBMeshParams.modelToWorld, CBCameraParams.worldToCameraProj);
    output.position = mul(input.position, MVP);
    output.normal = normalize(mul(input.normal, CBMeshParams.modelToWorldNormal));

    output.tangent = normalize(mul(float4(input.tangent, 0.0f), CBMeshParams.modelToWorldNormal)).xyz;
    output.bitangent = normalize(mul(float4(input.bitangent, 0.0f), CBMeshParams.modelToWorldNormal)).xyz;
    output.texcoord0 = input.texcoord0;
    output.color0 = input.color0;
    float4 worldPos = mul(input.position, CBMeshParams.modelToWorld);
    output.worldPos = worldPos.xyz / worldPos.w;
    return output;
}
