#pragma  once

#include "Scene.h"

class SceneRenderer {
public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) noexcept;
    void SetTarget(ID3D11RenderTargetView* RTV, uint32_t w, uint32_t h) noexcept;
    void RenderFrame(const Scene* scene) noexcept;

protected:
    void RenderMesh(const Scene* scene, const Mesh& mesh) noexcept;
    void UploadCameraParams() noexcept;
    void UploadMeterialParams(const Scene* scene) noexcept;
    void UploadPointLights(const Scene* scene) noexcept;

protected:
    ID3D11Device* m_Device = nullptr;
    ID3D11DeviceContext* m_Ctx = nullptr;

    ID3D11RasterizerState* m_RasterizerState = nullptr;

    ID3D11Buffer* m_CameraParamsCB = nullptr;
    ID3D11Buffer* m_MaterialParamsCB = nullptr;
    ID3D11Buffer* m_PointLights = nullptr;
    ID3D11ShaderResourceView* m_PointLightsSRV = nullptr;

    ID3D11RenderTargetView* m_RTV = nullptr;
    uint32_t m_TargetW = 0;
    uint32_t m_TargetH = 0;

    ID3D11DepthStencilView* m_DSV = nullptr;
    ID3D11Texture2D* m_DSVTexture = nullptr;
};
