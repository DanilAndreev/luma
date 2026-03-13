#ifndef __LUMA_HLSL_HLSLCPPCOMPAT__
#define __LUMA_HLSL_HLSLCPPCOMPAT__

#ifdef __cplusplus
#define LUMA_CPP_ONLY(exp) exp
#else
#define LUMA_CPP_ONLY(exp)
#endif

LUMA_CPP_ONLY(using float4 = DirectX::XMFLOAT4);
LUMA_CPP_ONLY(using float4x4 = DirectX::XMFLOAT4X4);


#endif // __LUMA_HLSL_HLSLCPPCOMPAT__