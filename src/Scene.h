#pragma once

#include <DirectXPackedVector.h>

constexpr size_t VertexAttributesMaxTexCoords = 8;
constexpr size_t VertexAttributesMaxColors = 8;

enum class VertexAttributesMask : uint64_t {
    None = 0x0, // Vertex has only float4 position per item
    Normals = 0x1 << 0, // float4 normal vector
    TexCoords0 = 0x1 << 1, // float2 texture coordinetes
    TexCoords1 = 0x1 << 2, // float2 texture coordinetes
    TexCoords2 = 0x1 << 3, // float2 texture coordinetes
    TexCoords3 = 0x1 << 4, // float2 texture coordinetes
    TexCoords4 = 0x1 << 5, // float2 texture coordinetes
    TexCoords5 = 0x1 << 6, // float2 texture coordinetes
    TexCoords6 = 0x1 << 7, // float2 texture coordinetes
    TexCoords7 = 0x1 << 8, // float2 texture coordinetes
    Color0 = 0x1 << 9, // float4 color
    Color1 = 0x1 << 10, // float4 color
    Color2 = 0x1 << 11, // float4 color
    Color3 = 0x1 << 12, // float4 color
    Color4 = 0x1 << 13, // float4 color
    Color5 = 0x1 << 14, // float4 color
    Color6 = 0x1 << 15, // float4 color
    Color7 = 0x1 << 16, // float4 color
    ValidMask = 0xFFFF1,
};
LUMA_DEFINE_BITMASK_ENUM(VertexAttributesMask);
constexpr size_t VertexAttributesEntriesCount = std::popcount(static_cast<uint64_t>(VertexAttributesMask::ValidMask));

constexpr size_t VertexAttributesEntriesSizeDefault[VertexAttributesEntriesCount] = {
    // sizeof(DirectX::XMFLOAT4) - Float4 position is always first
    sizeof(DirectX::XMFLOAT4), // Normals
    sizeof(DirectX::XMFLOAT2), // TexCoords0
    sizeof(DirectX::XMFLOAT2), // TexCoords1
    sizeof(DirectX::XMFLOAT2), // TexCoords2
    sizeof(DirectX::XMFLOAT2), // TexCoords3
    sizeof(DirectX::XMFLOAT2), // TexCoords4
    sizeof(DirectX::XMFLOAT2), // TexCoords5
    sizeof(DirectX::XMFLOAT2), // TexCoords6
    sizeof(DirectX::XMFLOAT2), // TexCoords7
    sizeof(DirectX::XMFLOAT4), // Color0
    sizeof(DirectX::XMFLOAT4), // Color1
    sizeof(DirectX::XMFLOAT4), // Color2
    sizeof(DirectX::XMFLOAT4), // Color3
    sizeof(DirectX::XMFLOAT4), // Color4
    sizeof(DirectX::XMFLOAT4), // Color5
    sizeof(DirectX::XMFLOAT4), // Color6
    sizeof(DirectX::XMFLOAT4), // Color7
};

//TODO: align everything to 4/2/1
constexpr size_t VertexAttributesEntriesSizeHalf[VertexAttributesEntriesCount] = {
    sizeof(DirectX::PackedVector::XMHALF4), // Normals
    sizeof(DirectX::PackedVector::XMHALF2), // TexCoords0
    sizeof(DirectX::PackedVector::XMHALF2), // TexCoords1
    sizeof(DirectX::PackedVector::XMHALF2), // TexCoords2
    sizeof(DirectX::PackedVector::XMHALF2), // TexCoords3
    sizeof(DirectX::PackedVector::XMHALF2), // TexCoords4
    sizeof(DirectX::PackedVector::XMHALF2), // TexCoords5
    sizeof(DirectX::PackedVector::XMHALF2), // TexCoords6
    sizeof(DirectX::PackedVector::XMHALF2), // TexCoords7
    sizeof(DirectX::PackedVector::XMHALF4), // Color0
    sizeof(DirectX::PackedVector::XMHALF4), // Color1
    sizeof(DirectX::PackedVector::XMHALF4), // Color2
    sizeof(DirectX::PackedVector::XMHALF4), // Color3
    sizeof(DirectX::PackedVector::XMHALF4), // Color4
    sizeof(DirectX::PackedVector::XMHALF4), // Color5
    sizeof(DirectX::PackedVector::XMHALF4), // Color6
    sizeof(DirectX::PackedVector::XMHALF4), // Color7
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
    //TODO: add UNORM color
    HalfColor0 = 0x1 << 9,
    HalfColor1 = 0x1 << 10,
    HalfColor2 = 0x1 << 11,
    HalfColor3 = 0x1 << 12,
    HalfColor4 = 0x1 << 13,
    HalfColor5 = 0x1 << 14,
    HalfColor6 = 0x1 << 15,
    HalfColor7 = 0x1 << 16,
    validMask = 0xFFFF1,
};
LUMA_DEFINE_BITMASK_ENUM(VertexAttributesFlags);

/**
  * @warning Includes first Float4 position size
  */
inline constexpr size_t ResolveVAOffsetFromMask(size_t vaIdx, VertexAttributesMask mask, VertexAttributesFlags flags) noexcept {
    size_t result = sizeof(DirectX::XMFLOAT4);
    for (size_t i = 0; i < vaIdx; ++i) {
        if (bool(mask & static_cast<VertexAttributesMask>(1 << i))) {
            const auto pred = bool(flags & static_cast<VertexAttributesFlags>(1 << i));
            result += pred ? VertexAttributesEntriesSizeHalf[i] : VertexAttributesEntriesSizeDefault[i];
        }
    }
    return result;
}

struct Mesh {
    VertexAttributesMask vaMask = VertexAttributesMask::None;
    VertexAttributesFlags vaFlags = {};

    size_t vaStride = 0;
    std::vector<char> vertices;
    std::vector<uint32_t> indices;
    DirectX::XMFLOAT4X4 transform;

    ID3D11Buffer* vb = nullptr;
    ID3D11Buffer* ib = nullptr;
    ID3D11InputLayout* inputLayout = nullptr;
};

struct MeshInstance {
    size_t meshIndex;
    DirectX::XMFLOAT4X4 transform;
    bool opaque;
};

struct DirectionalLight {
    DirectX::XMFLOAT3 ambientColor = {0.1f, 0.1f, 0.1f};
    DirectX::XMFLOAT3 diffuseColor = {1.0f, 1.0f, 1.0f};
    DirectX::XMFLOAT3 specularColor = {1.0f, 1.0f, 1.0f};
    DirectX::XMFLOAT3 direction = {-1.0f, -1.0f, -1.0f};
    float intensity = 0.3;

    DirectX::XMFLOAT4X4 worldToLightProj = {};
    DirectX::XMFLOAT4X4 worldToLight = {};
    DirectX::XMFLOAT4X4 lightToProj = {};
};

struct PointLight {
    DirectX::XMFLOAT3 ambientColor = {0.1f, 0.1f, 0.1f};
    DirectX::XMFLOAT3 diffuseColor = {1.0f, 1.0f, 1.0f};
    DirectX::XMFLOAT3 specularColor = {1.0f, 1.0f, 1.0f};
    DirectX::XMFLOAT3 position = {0.0f, 0.0f, 0.0f};

    float constantAttenuation = 1.0f;
    float linearAttenuation = 0.14f;
    float quadraticAttenuation = 0.07f;

    float shadowMapProjNearPlane = 1.0f;
    float shadowMapProjFarPlane = 25.0f;

    DirectX::XMMATRIX shadowmapProj;
    DirectX::XMMATRIX shadowmapProjInv;
};

struct Scene {
    std::vector<Mesh> meshes{};
    std::vector<MeshInstance> instances{};
    std::vector<PointLight> pointLights{};
    DirectionalLight directionalLight{};
};

