#include "ShaderTypes.hlsli"

cbuffer CB_CameraParams : register(b0) { HLSL::CameraParams CBCameraParams; }
StructuredBuffer<HLSL::PointLight> SRVPointLight : register(t0);

struct VSOut {
    float4 position: SV_Position;
    float3 color: COLOR;
};

struct PSOut {
    float4 color: SV_Target;
};

VSOut VSMain(uint vtID: SV_VertexID, uint iID: SV_InstanceID) {
    float3 vertices[] ={
        //front
        float3(0.2f, 0.2f, 0.0f),    // top right
        float3(0.2f, -0.2f, 0.0f),   // bottom right
        float3(-0.2f, -0.2f, 0.0f),  // bottom left
        float3(-0.2f, 0.2f, 0.0f),   // top left

        //back
        float3(0.2f, 0.2f, -0.4f),   // top right
        float3(0.2f, -0.2f, -0.4f),  // bottom right
        float3(-0.2f, -0.2f, -0.4f), // bottom left
        float3(-0.2f, 0.2f, -0.4f),  // top left
    };

    uint indices[] = {
        // front  (normal +Z)
        0, 3, 1,
        1, 3, 2,
        // back   (normal -Z)
        4, 5, 7,
        5, 6, 7,
        // right  (normal +X)
        0, 1, 4,
        1, 5, 4,
        // left   (normal -X)
        2, 3, 6,
        3, 7, 6,
        // top    (normal +Y)
        0, 4, 3,
        4, 7, 3,
        // bottom (normal -Y)
        1, 2, 5,
        2, 6, 5
    };

    const float size = 0.1f;
    float4x4 viewProj = mul(CBCameraParams.worldToCamera, CBCameraParams.cameraToProjection);

    VSOut output;
    float3 cubeVertex = vertices[indices[vtID]] * size;

    output.position = float4(cubeVertex, 1.0f) + float4(SRVPointLight[iID].position.xyz, 0.0f);
    output.position = mul(output.position, viewProj);
    output.color = SRVPointLight[iID].color;
    return output;
}

PSOut PSMain(VSOut input) {
    PSOut output;
    output.color = float4(input.color, 1.0f);
    return output;
}
