#pragma once
#include "Scene.h"

namespace Loader {
    bool LoadAssetsToScene(Scene& scene, const std::filesystem::path& filepath) noexcept;

    bool UploadSceneBuffersToGPU(Scene& scene, ID3D11Device* device) noexcept;
}