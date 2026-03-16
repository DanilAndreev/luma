#include "vertexSemantics.hlsli"
#include "ShaderTypes.hlsli"

StructuredBuffer<HLSL::PointLight> SRVPointLight : register(t0);
cbuffer CB_CurShadowPointLight : register(b0) { HLSL::PointLight CBCurShadowPointLight; }

struct PSOut
{
    float depth : SV_Depth;
};

PSOut PSMain(VSOut input) {
    float lightDistance = length(input.worldPos - CBCurShadowPointLight.position);
    PSOut output;
    output.depth = lightDistance / CBCurShadowPointLight.shadowMapProjFarPlane;
    return output;
}