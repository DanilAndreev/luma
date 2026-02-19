#pragma once

#include <DirectXMath.h>

struct Mesh{
    //TODO: maybe make vertex attributes more cache-local
    std::vector<DirectX::XMFLOAT4> vertices;
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

