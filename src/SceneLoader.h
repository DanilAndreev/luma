#pragma once
#include "Scene.h"

namespace Loader {
    bool LoadScene(Scene& scene) noexcept;

    bool UploadSceneBuffersToGPU(Scene& scene, ID3D11Device* device) noexcept;
}