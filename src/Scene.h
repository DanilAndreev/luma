#pragma once

#include <DirectXMath.h>

constexpr size_t VertexAttributesMaxTexCoords = 8;
constexpr size_t VertexAttributesMaxColors = 8;

enum class VertexAttributesMask : uint64_t {
    None = 0x0, // Vertex has only float4 position per item
    Normals = 0x1 << 0, // float3 normal vector
    TexCoords0 = 0x1 << 1, // float2 texture coordinetes
    TexCoords1 = 0x1 << 2, // float2 texture coordinetes
    TexCoords2 = 0x1 << 3, // float2 texture coordinetes
    TexCoords3 = 0x1 << 4, // float2 texture coordinetes
    TexCoords4 = 0x1 << 5, // float2 texture coordinetes
    TexCoords5 = 0x1 << 6, // float2 texture coordinetes
    TexCoords6 = 0x1 << 7, // float2 texture coordinetes
    TexCoords7 = 0x1 << 8, // float2 texture coordinetes
    Color0 = 0x1 << 9, // float3 color
    Color1 = 0x1 << 10, // float3 color
    Color2 = 0x1 << 11, // float3 color
    Color3 = 0x1 << 12, // float3 color
    Color4 = 0x1 << 13, // float3 color
    Color5 = 0x1 << 14, // float3 color
    Color6 = 0x1 << 15, // float3 color
    Color7 = 0x1 << 16, // float3 color
};

enum class VertexAttributesFlags : uint32_t {
    HalfNormals = 0x1 << 0,
    HalfTexCoords0 = 0x1 << 1,
    HalfTexCoords1 = 0x1 << 2,
    HalfTexCoords2 = 0x1 << 3,
    HalfTexCoords3 = 0x1 << 4,
    HalfTexCoords4 = 0x1 << 5,
    HalfTexCoords5 = 0x1 << 6,
    HalfTexCoords6 = 0x1 << 7,
    HalfTexCoords7 = 0x1 << 8,
    HalfColor0 = 0x1 << 9,
    HalfColor1 = 0x1 << 10,
    HalfColor2 = 0x1 << 11,
    HalfColor3 = 0x1 << 12,
    HalfColor4 = 0x1 << 13,
    HalfColor5 = 0x1 << 14,
    HalfColor6 = 0x1 << 15,
    HalfColor7 = 0x1 << 16,
};

struct Mesh{
    VertexAttributesMask vaMask = VertexAttributesMask::None;
    VertexAttributesFlags vaFlags = {};

    std::vector<char> vertices;
    std::vector<uint32_t> indices;
    DirectX::XMFLOAT4X4 transform;
};

struct MeshInstance {
    size_t meshIndex;
    DirectX::XMFLOAT4X4 transform;
    bool opaque;
};

struct PointLight {
    DirectX::XMFLOAT4 color;
    DirectX::XMFLOAT4 position;
    float attenuation;
};

struct Scene {
    std::vector<Mesh> meshes{};
    std::vector<MeshInstance> instances{};
    std::vector<PointLight> pointLights{};
};

