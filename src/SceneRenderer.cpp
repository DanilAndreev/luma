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

        desc.ByteWidth = sizeof(HLSL::MeshParams);
        device->CreateBuffer(&desc, nullptr, &m_MeshParamsCB);

        desc.ByteWidth = sizeof(HLSL::LightParams);
        device->CreateBuffer(&desc, nullptr, &m_LightParamsCB);
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
    UploadLightParams(scene);

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
    UploadMeshParams(scene, mesh);

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

    ID3D11Buffer* VSCBs[] = {m_CameraParamsCB, m_MeshParamsCB};
    m_Ctx->VSSetConstantBuffers(0, std::size(VSCBs), VSCBs);

    ID3D11Buffer* PSCBs[] = {m_CameraParamsCB, m_MeshParamsCB, m_LightParamsCB};
    m_Ctx->PSSetConstantBuffers(0, std::size(PSCBs), PSCBs);
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

    m_Ctx->DrawInstanced(36, scene->pointLights.size(), 0, 0);
}

void SceneRenderer::UploadCameraParams() noexcept {
    using namespace DirectX;
    HLSL::CameraParams params{};
    XMFLOAT4X4 worldToCamera = g_Cam.ViewTransform();
    float aspectRatio = static_cast<float>(m_TargetW) / static_cast<float>(m_TargetH);
    XMFLOAT4X4 cameraToProjection = g_Cam.CameraToProjection(aspectRatio);
    XMStoreFloat4x4(&params.worldToCamera, XMMatrixTranspose(XMLoadFloat4x4(&worldToCamera)));
    XMStoreFloat4x4(&params.cameraToProjection, XMMatrixTranspose(XMLoadFloat4x4(&cameraToProjection)));
    params.worldPos = g_Cam.GetPosition();
    m_Ctx->UpdateSubresource(m_CameraParamsCB, 0, nullptr, &params, sizeof(params), 0);
}

void SceneRenderer::UploadMeshParams(const Scene* scene, const Mesh& mesh) noexcept {
    using namespace DirectX;
    HLSL::MeshParams params{};
    params.material.shininess = 32.0f;
    XMMATRIX transform = XMLoadFloat4x4(&mesh.transform);
    XMStoreFloat4x4(&params.transform, XMMatrixTranspose(transform));
    m_Ctx->UpdateSubresource(m_MeshParamsCB, 0, nullptr, &params, sizeof(params), 0);
}

void SceneRenderer::UploadLightParams(const Scene *scene) noexcept {
    using namespace DirectX;
    HLSL::LightParams params{};
    params.pointLightCount = scene->pointLights.size();
    params.dirLight.ambientColor = scene->directionalLight.ambientColor;
    params.dirLight.diffuseColor = scene->directionalLight.diffuseColor;
    params.dirLight.specularColor = scene->directionalLight.specularColor;
    params.dirLight.direction = scene->directionalLight.direction;
    m_Ctx->UpdateSubresource(m_LightParamsCB, 0, nullptr, &params, sizeof(params), 0);
}

void SceneRenderer::UploadPointLights(const Scene *scene) noexcept {
    if (scene->pointLights.size() == 0)
        return;

    std::vector<HLSL::PointLight> lights{};
    lights.resize(scene->pointLights.size());
    for (size_t i = 0; i < scene->pointLights.size(); ++i) {
        lights[i].position = scene->pointLights[i].position;
        lights[i].ambientColor = scene->pointLights[i].ambientColor;
        lights[i].diffuseColor = scene->pointLights[i].diffuseColor;
        lights[i].specularColor = scene->pointLights[i].specularColor;
        lights[i].constantAttenuation = scene->pointLights[i].constantAttenuation;
        lights[i].linearAttenuation = scene->pointLights[i].linearAttenuation;
        lights[i].quadraticAttenuation = scene->pointLights[i].quadraticAttenuation;
    }
    m_Ctx->UpdateSubresource(m_PointLights, 0, nullptr, lights.data(), lights.size() * sizeof(lights[0]), 0);
}
