#include "SceneRenderer.h"
#include "hlsl/ShaderTypes.hlsli"

void SceneRenderer::Initialize(ID3D11Device *device, ID3D11DeviceContext *context) noexcept {
    m_Device = device;
    m_Ctx = context;

    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.ScissorEnable = false;
    rasterizerDesc.AntialiasedLineEnable = false;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.MultisampleEnable = false;

    m_Device->CreateRasterizerState(&rasterizerDesc, &m_RasterizerState);

    {
        D3D11_BUFFER_DESC desc{};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = 0;
        desc.ByteWidth = sizeof(HLSL::CameraParams);
        desc.StructureByteStride = 0;
        device->CreateBuffer(&desc, nullptr, &m_CameraParamsCB);

        desc.ByteWidth = sizeof(HLSL::MaterialParams);
        device->CreateBuffer(&desc, nullptr, &m_MaterialParamsCB);
    }

    {
        D3D11_BUFFER_DESC desc{};
        desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.ByteWidth = sizeof(HLSL::PointLight) * 128;
        desc.StructureByteStride = sizeof(HLSL::PointLight);
        device->CreateBuffer(&desc, nullptr, &m_PointLights);

        D3D11_SHADER_RESOURCE_VIEW_DESC view{};
        view.Format = DXGI_FORMAT_UNKNOWN;
        view.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        view.Buffer.ElementWidth = desc.ByteWidth / desc.StructureByteStride;
        device->CreateShaderResourceView(m_PointLights, &view, &m_PointLightsSRV);
    }
}

void SceneRenderer::SetTarget(ID3D11RenderTargetView *RTV, uint32_t w, uint32_t h) noexcept {
    m_RTV = RTV;
    m_TargetW = w;
    m_TargetH = h;

    {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.Width = m_TargetW;
        desc.Height = m_TargetH;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;

        m_Device->CreateTexture2D(&desc, nullptr, &m_DSVTexture);

        D3D11_DEPTH_STENCIL_VIEW_DESC view{};
        view.Format = desc.Format;
        view.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        view.Flags = 0;
        view.Texture2D.MipSlice = 0;
        m_Device->CreateDepthStencilView(m_DSVTexture, &view, &m_DSV);
    }

}

void SceneRenderer::RenderFrame(const Scene *scene) noexcept {
    m_Ctx->OMSetRenderTargets(1, &m_RTV, m_DSV);

    UploadCameraParams();
    UploadPointLights(scene);

    m_Ctx->ClearDepthStencilView(m_DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    D3D11_VIEWPORT viewport{};
    viewport.Width = m_TargetW;
    viewport.Height = m_TargetH;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    m_Ctx->RSSetViewports(1, &viewport);

    for (size_t i = 0; i < scene->meshes.size(); ++i) {
        RenderMesh(scene, scene->meshes[i]);
    }

    VisualizePointLight(scene, 0);
}

struct MeshIACache {
    //TODO: maybe move stides, array equal elements
    UINT vaStrides[VertexAttributesEntriesCount + 1] = {};
    UINT vaOffsets[VertexAttributesEntriesCount + 1] = {};
};

void SceneRenderer::RenderMesh(const Scene* scene, const Mesh& mesh) noexcept {
    UploadMeterialParams(scene);

    m_Ctx->VSSetShader(g_SM.Get(VertexShaderID::Unity), nullptr, 0);
    m_Ctx->PSSetShader(g_SM.Get(PixelShaderID::Unity), nullptr, 0);

    ID3D11Buffer* vertexBuffers[VertexAttributesEntriesCount] = {};
    MeshIACache meshIA = {};

    vertexBuffers[0] = mesh.vb;
    meshIA.vaOffsets[0] = 0;
    meshIA.vaStrides[0] = mesh.vaStride;

    size_t vaActiveIdx = 0;
    for (size_t i = 0; i < VertexAttributesEntriesCount; ++i) {
        if (bool(mesh.vaMask & static_cast<VertexAttributesMask>(1 << i))) {
            vertexBuffers[i + 1] = mesh.vb;
            meshIA.vaOffsets[i + 1] = ResolveVAOffsetFromMask(i, mesh.vaMask, mesh.vaFlags);
        }
        meshIA.vaStrides[i + 1] = mesh.vaStride;
    }
    m_Ctx->IASetVertexBuffers(0, VertexAttributesEntriesCount, vertexBuffers, meshIA.vaStrides, meshIA.vaOffsets);
    m_Ctx->IASetIndexBuffer(mesh.ib, DXGI_FORMAT_R32_UINT, 0);

    m_Ctx->IASetInputLayout(mesh.inputLayout);
    m_Ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_Ctx->VSSetConstantBuffers(0, 1, &m_CameraParamsCB);

    m_Ctx->PSSetConstantBuffers(0, 1, &m_MaterialParamsCB);
    m_Ctx->PSSetShaderResources(0, 1, &m_PointLightsSRV);

    m_Ctx->RSSetState(m_RasterizerState);

    m_Ctx->DrawIndexed(mesh.indices.size(), 0, 0);
}

void SceneRenderer::VisualizePointLight(const Scene *scene, size_t lightID) noexcept {
    m_Ctx->VSSetShader(g_SM.Get(VertexShaderID::PointLightVisualize), nullptr, 0);
    m_Ctx->PSSetShader(g_SM.Get(PixelShaderID::PointLightVisualize), nullptr, 0);

    m_Ctx->VSSetConstantBuffers(0, 1, &m_CameraParamsCB);
    m_Ctx->VSSetShaderResources(0, 1, &m_PointLightsSRV);

    m_Ctx->PSSetShaderResources(0, 1, &m_PointLightsSRV);

    m_Ctx->RSSetState(m_RasterizerState);

    m_Ctx->Draw(36, 0);
}

void SceneRenderer::UploadCameraParams() noexcept {
    using namespace DirectX;
    HLSL::CameraParams params{};
    XMFLOAT4X4 worldToCamera = g_Cam.ViewTransform();
    float aspectRatio = static_cast<float>(m_TargetW) / static_cast<float>(m_TargetH);
    XMFLOAT4X4 cameraToProjection = g_Cam.CameraToProjection(aspectRatio);
    XMStoreFloat4x4(&params.worldToCamera, XMMatrixTranspose(XMLoadFloat4x4(&worldToCamera)));
    XMStoreFloat4x4(&params.cameraToProjection, XMMatrixTranspose(XMLoadFloat4x4(&cameraToProjection)));
    m_Ctx->UpdateSubresource(m_CameraParamsCB, 0, nullptr, &params, sizeof(params), 0);
}

void SceneRenderer::UploadMeterialParams(const Scene* scene) noexcept {
    using namespace DirectX;
    HLSL::MaterialParams params{};
    params.ambientColor = {1.0f, 0.0f, 0.0f, 1.0f};
    // params.specularColor = {0.0f, 1.0f, 0.0f, 1.0f};
    // params.phongColor = {0.0f, 0.0f, 1.0f, 1.0f};
    params.ambientStrength = 0.1f;
    params.pointLightCount = scene->pointLights.size();
    // params.shininess = 16.0f;
    m_Ctx->UpdateSubresource(m_MaterialParamsCB, 0, nullptr, &params, sizeof(params), 0);
}

void SceneRenderer::UploadPointLights(const Scene *scene) noexcept {
    if (scene->pointLights.size() == 0)
        return;

    std::vector<HLSL::PointLight> lights{};
    lights.resize(scene->pointLights.size());
    for (size_t i = 0; i < scene->pointLights.size(); ++i) {
        lights[i].position = scene->pointLights[i].position;
        lights[i].color = scene->pointLights[i].color;
        lights[i].shininess = 16;
    }
    m_Ctx->UpdateSubresource(m_PointLights, 0, nullptr, lights.data(), lights.size() * sizeof(lights[0]), 0);
}
