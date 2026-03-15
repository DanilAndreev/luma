#include "vertexSemantics.hlsli"

#include "ShaderTypes.hlsli"

cbuffer CB_CameraParams : register(b0) { HLSL::CameraParams CBCameraParams; }
cbuffer CB_MeshParams : register(b1) { HLSL::MeshParams CBMeshParams; }
cbuffer CB_LightParams : register(b2) { HLSL::LightParams CBLightParams; }

VSOut VSMain(VSIn input) {
    VSOut output;

    float4x4 viewProj = mul(mul(CBMeshParams.transform, CBCameraParams.worldToCamera), CBCameraParams.cameraToProjection);
    output.position = mul(input.position, viewProj);
    //TODO: calculate normal matrix and transform normals.
    output.normal = input.normal;
    //output.texcoor0 = input.texcoor0;
    //output.color0 = input.color0;
    output.worldPos = mul(input.position, CBMeshParams.transform).xyz;



    // output.lightPos = mul(input.position, CBLightParams.dirLight.worldToLightProj);
    output.lightPos = mul(input.position, CBMeshParams.transform);
    output.lightPos = mul(output.lightPos, CBLightParams.dirLight.worldToLight);
    output.lightPos = mul(output.lightPos, CBLightParams.dirLight.lightToProj);
    return output;
}
