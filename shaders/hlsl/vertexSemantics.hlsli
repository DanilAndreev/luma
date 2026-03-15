#ifndef __LUMA_HLSL_VERTEX_SEMANTICS__
#define __LUMA_HLSL_VERTEX_SEMANTICS__

struct VSIn
{
    float4 position : POSITION;
    float4 normal : NORMAL;
//    float4 texcoor0 : TEXCOORD0;
//    float4 color0 : COLOR0;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
//    float4 texcoor0 : TEXCOORD0;
//    float4 color0 : COLOR0;
    float3 worldPos: POSISION;
    float4 lightPos: POSITION2;
};

#endif // __LUMA_HLSL_VERTEX_SEMANTICS__
