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

    D3D11_BUFFER_DESC desc{};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.ByteWidth = sizeof(HLSL::CameraParams);
    desc.StructureByteStride = 0;
    device->CreateBuffer(&desc, nullptr, &m_CameraParamsCB);

}

void SceneRenderer::RenderFrame(const Scene *scene, ID3D11RenderTargetView* RTV) noexcept {
    m_Ctx->OMSetRenderTargets(1, &RTV, nullptr);

    UploadCameraParams();

    for (size_t i = 0; i < scene->meshes.size(); ++i) {
        RenderMesh(scene->meshes[i]);
    }
}

struct MeshIACache {
    //TODO: maybe move stides, array equal elements
    UINT vaStrides[VertexAttributesEntriesCount + 1] = {};
    UINT vaOffsets[VertexAttributesEntriesCount + 1] = {};
};

void SceneRenderer::RenderMesh(const Mesh& mesh) noexcept {
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

    D3D11_VIEWPORT viewport{};
    viewport.Width = 800;
    viewport.Height = 600;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    m_Ctx->RSSetViewports(1, &viewport);
    m_Ctx->RSSetState(m_RasterizerState);

    m_Ctx->DrawIndexed(mesh.indices.size(), 0, 0);
}

void SceneRenderer::UploadCameraParams() noexcept {
    HLSL::CameraParams cameraParams{};
    cameraParams.worldToCamera = g_Cam.ViewTransform();
    m_Ctx->UpdateSubresource(m_CameraParamsCB, 0, nullptr, &cameraParams, sizeof(cameraParams), 0);
}
