#pragma once
#include "Scene.h"

namespace Loader {
    bool LoadAssetsToScene(Scene& scene, const std::filesystem::path& filepath, DirectX::XMFLOAT4X4 transform) noexcept;

    bool UploadSceneBuffersToGPU(Scene& scene, ID3D11Device* device) noexcept;
}