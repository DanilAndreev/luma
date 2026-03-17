#pragma once

#include <DirectXPackedVector.h>

constexpr size_t VertexAttributesMaxTexCoords = 6;
constexpr size_t VertexAttributesMaxColors = 6;

enum class VertexAttributesMask : uint64_t {
    None = 0x0, // Vertex has only float4 position per item
    Normals = 0x1 << 0, // float4 normal vector
    Tangent = 0x1 << 1, // float4 tangent vector
    Bitangent = 0x1 << 2, // float4 bitangent vector
    TexCoords0 = 0x1 << 3, // float2 texture coordinates
    TexCoords1 = 0x1 << 4, // float2 texture coordinates
    TexCoords2 = 0x1 << 5, // float2 texture coordinates
    TexCoords3 = 0x1 << 6, // float2 texture coordinates
    TexCoords4 = 0x1 << 7, // float2 texture coordinates
    TexCoords5 = 0x1 << 8, // float2 texture coordinates
    Color0 = 0x1 << 10, // float4 color
    Color1 = 0x1 << 11, // float4 color
    Color2 = 0x1 << 12, // float4 color
    Color3 = 0x1 << 13, // float4 color
    Color4 = 0x1 << 14, // float4 color
    Color5 = 0x1 << 15, // float4 color
    ValidMask = 0xFFFF,
};
LUMA_DEFINE_BITMASK_ENUM(VertexAttributesMask);
constexpr size_t VertexAttributesEntriesCount = std::popcount(static_cast<uint64_t>(VertexAttributesMask::ValidMask));

constexpr size_t VertexAttributesEntriesSizeDefault[VertexAttributesEntriesCount] = {
    // sizeof(DirectX::XMFLOAT4) - Float4 position is always first
    sizeof(DirectX::XMFLOAT3), // Normal
    sizeof(DirectX::XMFLOAT3), // Tangent
    sizeof(DirectX::XMFLOAT3), // Bitangent
    sizeof(DirectX::XMFLOAT2), // TexCoords0
    sizeof(DirectX::XMFLOAT2), // TexCoords1
    sizeof(DirectX::XMFLOAT2), // TexCoords2
    sizeof(DirectX::XMFLOAT2), // TexCoords3
    sizeof(DirectX::XMFLOAT2), // TexCoords4
    sizeof(DirectX::XMFLOAT2), // TexCoords5
    sizeof(DirectX::XMFLOAT4), // Color0
    sizeof(DirectX::XMFLOAT4), // Color1
    sizeof(DirectX::XMFLOAT4), // Color2
    sizeof(DirectX::XMFLOAT4), // Color3
    sizeof(DirectX::XMFLOAT4), // Color4
    sizeof(DirectX::XMFLOAT4), // Color5
};

//TODO: align everything to 4/2/1
constexpr size_t VertexAttributesEntriesSizeHalf[VertexAttributesEntriesCount] = {
    sizeof(DirectX::PackedVector::XMHALF4), // Normal
    sizeof(DirectX::PackedVector::XMHALF4), // Tangent
    sizeof(DirectX::PackedVector::XMHALF4), // Bitangent
    sizeof(DirectX::PackedVector::XMHALF2), // TexCoords0
    sizeof(DirectX::PackedVector::XMHALF2), // TexCoords1
    sizeof(DirectX::PackedVector::XMHALF2), // TexCoords2
    sizeof(DirectX::PackedVector::XMHALF2), // TexCoords3
    sizeof(DirectX::PackedVector::XMHALF2), // TexCoords4
    sizeof(DirectX::PackedVector::XMHALF2), // TexCoords5
    sizeof(DirectX::PackedVector::XMHALF4), // Color0
    sizeof(DirectX::PackedVector::XMHALF4), // Color1
    sizeof(DirectX::PackedVector::XMHALF4), // Color2
    sizeof(DirectX::PackedVector::XMHALF4), // Color3
    sizeof(DirectX::PackedVector::XMHALF4), // Color4
    sizeof(DirectX::PackedVector::XMHALF4), // Color5
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
    std::string name = "UnnamedMesh";
    VertexAttributesMask vaMask = VertexAttributesMask::None;
    VertexAttributesFlags vaFlags = {};

    size_t vaStride = 0;
    std::vector<char> vertices;
    std::vector<uint32_t> indices;
    DirectX::XMFLOAT4X4 transform;

    ID3D11Buffer* vb = nullptr;
    ID3D11Buffer* ib = nullptr;
    ID3D11InputLayout* inputLayout = nullptr;

    size_t materialIdx;
};

struct MeshInstance {
    size_t meshIndex;
    DirectX::XMFLOAT4X4 transform;
    bool opaque;
};

struct Material {
    DirectX::XMFLOAT3 color = {0.7f, 0.7f, 0.7f};
    float shininess = 32.0f;

    // TODO: move to scene texture pool for de-duplication.
    //       Also float3 and float textures could be joined to float4 and split with views.
    ID3D11Texture2D* diffuseMap = nullptr;
    ID3D11Texture2D* specularMap = nullptr;
    ID3D11Texture2D* normalMap = nullptr;
    ID3D11Texture2D* heightMap = nullptr;

    ID3D11ShaderResourceView* diffuseMapSRV = nullptr;
    ID3D11ShaderResourceView* specularMapSRV = nullptr;
    ID3D11ShaderResourceView* normalMapSRV = nullptr;
    ID3D11ShaderResourceView* heightMapSRV = nullptr;
};

struct DirectionalLight {
    DirectX::XMFLOAT3 color = {1.0f, 1.0f, 0.5f};
    DirectX::XMFLOAT3 direction = {-1.0f, -1.0f, -1.0f};
    float ambientIntensity = 0.1;
    float intensity = 0.3;

    DirectX::XMFLOAT4X4 worldToLightProj = {};
    DirectX::XMFLOAT4X4 worldToLight = {};
    DirectX::XMFLOAT4X4 lightToProj = {};
};

struct PointLight {
    DirectX::XMFLOAT3 color = {1.0f, 1.0f, 1.0f};
    DirectX::XMFLOAT3 position = {0.0f, 0.0f, 0.0f};

    float ambientIntensity = 0.1;
    float constantAttenuation = 1.0f;
    float linearAttenuation = 0.14f;
    float quadraticAttenuation = 0.07f;

    float shadowMapProjNearPlane = 1.0f;
    float shadowMapProjFarPlane = 25.0f;

    DirectX::XMMATRIX shadowmapProj = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX shadowmapProjInv = DirectX::XMMatrixIdentity();
};

struct Scene {
    std::vector<Mesh> meshes{};
    std::vector<MeshInstance> instances{};
    std::vector<Material> materials{Material{}};
    std::vector<PointLight> pointLights{};
    DirectionalLight directionalLight{};
};
