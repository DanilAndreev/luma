#pragma  once

#include "Scene.h"

class SceneRenderer {
public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) noexcept;
    void RenderFrame(const Scene* scene, ID3D11RenderTargetView* RTV) noexcept;

protected:
    void RenderMesh(const Mesh& mesh) noexcept;
    void UploadCameraParams() noexcept;

protected:
    ID3D11Device* m_Device = nullptr;
    ID3D11DeviceContext* m_Ctx = nullptr;

    ID3D11RasterizerState* m_RasterizerState = nullptr;

    ID3D11Buffer* m_CameraParamsCB = nullptr;
};
