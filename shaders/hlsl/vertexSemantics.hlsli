#ifndef __LUMA_HLSL_VERTEX_SEMANTICS__
#define __LUMA_HLSL_VERTEX_SEMANTICS__

struct VSIn
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float3 tangent : TANGENT0;
    float3 bitangent : TANGENT1;
    float4 texcoor0 : TEXCOORD0;
    float4 color0 : COLOR0;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float3 tangent : TANGENT0;
    float3 bitangent : TANGENT1;
    float4 texcoor0 : TEXCOORD0;
    float4 color0 : COLOR0;
    float3 worldPos: POSISION;
};

#endif // __LUMA_HLSL_VERTEX_SEMANTICS__
