#pragma  once

#include "Scene.h"

class SceneRenderer {
public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) noexcept;
    void SetTarget(ID3D11RenderTargetView* RTV, uint32_t w, uint32_t h) noexcept;
    void RenderFrame(Scene* scene) noexcept;

protected:
    void DirLightShadowPass(Scene* scene) noexcept;

    void RenderMesh(const Scene* scene, const Mesh& mesh, bool depthOnly = false) noexcept;
    void VisualizePointLight(const Scene* scene, size_t lightID) noexcept;

    void UploadCameraParams() noexcept;
    void UploadMeshParams(const Scene* scene, const Mesh& mesh) noexcept;
    void UploadLightParams(const Scene* scene) noexcept;
    void UploadPointLights(const Scene* scene) noexcept;

protected:
    static constexpr size_t SHADOW_MAP_DIM = 8192;

    ID3D11Device* m_Device = nullptr;
    ID3D11DeviceContext* m_Ctx = nullptr;

    ID3D11RasterizerState* m_RasterizerState = nullptr;

    ID3D11Buffer* m_CameraParamsCB = nullptr;
    ID3D11Buffer* m_MeshParamsCB = nullptr;
    ID3D11Buffer* m_LightParamsCB = nullptr;
    ID3D11Buffer* m_PointLights = nullptr;
    ID3D11ShaderResourceView* m_PointLightsSRV = nullptr;

    ID3D11RenderTargetView* m_RTV = nullptr;
    uint32_t m_TargetW = 0;
    uint32_t m_TargetH = 0;

    ID3D11Texture2D* m_DebugTex = nullptr;
    ID3D11RenderTargetView* m_DebugTexRTV = nullptr;

    ID3D11DepthStencilView* m_DSV = nullptr;
    ID3D11Texture2D* m_DSVTexture = nullptr;

    ID3D11Texture2D* m_DirLightShadowMapTex = nullptr;
    ID3D11DepthStencilView* m_DirLightShadowMapTexDSV = nullptr;
    ID3D11ShaderResourceView* m_DirLightShadowMapTexSRV = nullptr;
    ID3D11SamplerState* m_ShadowMapSMP = nullptr;
};
