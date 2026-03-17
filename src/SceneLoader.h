#pragma once
#include "Scene.h"

namespace Loader {
    bool LoadAssetsToScene(Scene& scene, const std::filesystem::path& filepath, DirectX::XMFLOAT4X4 transform) noexcept;

    bool UploadSceneBuffersToGPU(Scene& scene, ID3D11Device* device) noexcept;

    bool LoadTexture(const std::filesystem::path& path, ID3D11Device* device, ID3D11Texture2D** outTexture) noexcept;
}