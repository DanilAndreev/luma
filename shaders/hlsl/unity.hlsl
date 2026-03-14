#include "vertexSemantics.hlsli"
#include "ShaderTypes.hlsli"

cbuffer CB_MaterialParams : register(b0) { HLSL::MaterialParams CBMaterialParams; }
StructuredBuffer<HLSL::PointLight> SRVPointLight : register(t0);

struct PSOut
{
    float4 color : SV_Target;
};


PSOut PSMain(VSOut input) {
    PSOut output;
    output.color = CBMaterialParams.ambientColor * CBMaterialParams.ambientStrength;
    output.color += SRVPointLight[0].color;

    return output;
}