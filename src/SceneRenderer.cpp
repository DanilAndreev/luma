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

        desc.ByteWidth = sizeof(HLSL::PointLight);
        device->CreateBuffer(&desc, nullptr, &m_CurShadowPointLightCB);
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

    {
        D3D11_SAMPLER_DESC desc{};
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        desc.BorderColor[0] = 1.0f;
        desc.MinLOD = 0;
        desc.MaxLOD = 0;
        m_Device->CreateSamplerState(&desc, &m_ShadowMapSMP);
    }

    {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Format = DXGI_FORMAT_R32_TYPELESS;
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.Width = LUMA_DIR_SHADOW_MAP_DIM;
        desc.Height = LUMA_DIR_SHADOW_MAP_DIM;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;

        m_Device->CreateTexture2D(&desc, nullptr, &m_DirLightShadowMapTex);

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Flags = 0;
        dsvDesc.Texture2D.MipSlice = 0;
        m_Device->CreateDepthStencilView(m_DirLightShadowMapTex, &dsvDesc, &m_DirLightShadowMapTexDSV);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D10_1_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        m_Device->CreateShaderResourceView(m_DirLightShadowMapTex, &srvDesc, &m_DirLightShadowMapTexSRV);
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

    {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Format = DXGI_FORMAT_R32G32B32A32_TYPELESS;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.Width = m_TargetW;
        desc.Height = m_TargetH;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;

        m_Device->CreateTexture2D(&desc, nullptr, &m_DebugTex);

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
        rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        m_Device->CreateRenderTargetView(m_DebugTex, &rtvDesc, &m_DebugTexRTV);

    }
}

void SceneRenderer::InitShadowmapResources(size_t maxDirectionalPointLight) noexcept {
    if (!maxDirectionalPointLight) return;
    m_MaxDirectionalPointLight = maxDirectionalPointLight;

    D3D11_TEXTURE2D_DESC desc{};
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    desc.Width = LUMA_OMNIDIR_SHADOW_MAP_DIM;
    desc.Height = LUMA_OMNIDIR_SHADOW_MAP_DIM;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ArraySize = maxDirectionalPointLight * 6;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    m_Device->CreateTexture2D(&desc, nullptr, &m_PointLightShadowCubemapTexarr);

    //TODO: use GS to render it

    // D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    // dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    // dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    // dsvDesc.Flags = 0;
    // dsvDesc.Texture2DArray.MipSlice = 0;
    // dsvDesc.Texture2DArray.FirstArraySlice = 0;
    // dsvDesc.Texture2DArray.ArraySize = 1;
    // device->CreateDepthStencilView(light.m_ShadowCubemap, &dsvDesc, &light.m_ShadowCubemapDSV);

    m_PointLightShadowCubemapTexarrDSVs.resize(desc.ArraySize);
    for (size_t arrIdx = 0; arrIdx < desc.ArraySize; ++arrIdx) {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        dsvDesc.Flags = 0;
        dsvDesc.Texture2DArray.MipSlice = 0;
        dsvDesc.Texture2DArray.FirstArraySlice = arrIdx;
        dsvDesc.Texture2DArray.ArraySize = 1;
        m_Device->CreateDepthStencilView(m_PointLightShadowCubemapTexarr, &dsvDesc, &m_PointLightShadowCubemapTexarrDSVs[arrIdx]);
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D10_1_SRV_DIMENSION_TEXTURECUBEARRAY;
    srvDesc.TextureCubeArray.MipLevels = 1;
    srvDesc.TextureCubeArray.First2DArrayFace = 0;
    srvDesc.TextureCubeArray.NumCubes = maxDirectionalPointLight;
    m_Device->CreateShaderResourceView(m_PointLightShadowCubemapTexarr, &srvDesc, &m_PointLightShadowCubemapTexarrSRV);

}

void SceneRenderer::RenderFrame(Scene *scene) noexcept {
    assert(scene->pointLights.size() <= m_MaxDirectionalPointLight);
    DirLightShadowPass(scene);
    UploadPointLights(scene);
    PointLightShadowPass(scene);

    UploadPointLights(scene);
    UploadLightParams(scene);

    UploadCameraParams();

    ID3D11RenderTargetView* RTVs[] = {m_RTV, m_DebugTexRTV};

    FLOAT backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_Ctx->ClearRenderTargetView(m_DebugTexRTV, backgroundColor);
    m_Ctx->OMSetRenderTargets(std::size(RTVs), RTVs, m_DSV);
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

void SceneRenderer::DirLightShadowPass(Scene* scene) noexcept {
    {
        using namespace DirectX;
        XMVECTOR front = XMLoadFloat3(&scene->directionalLight.direction);
        XMVECTOR pos = XMVectorNegate(front);
        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        XMMATRIX worldToLight = XMMatrixLookToLH(pos, front, up);
        XMMATRIX lightToProjection = XMMatrixOrthographicLH(10, 10,0.001f, 10.0f);
        XMMATRIX worldToLightProj = XMMatrixMultiply(worldToLight, lightToProjection);

        HLSL::CameraParams params{};
        XMStoreFloat4x4(&params.worldToCamera, XMMatrixTranspose(worldToLight));
        XMStoreFloat4x4(&params.cameraToProjection, XMMatrixTranspose(lightToProjection));
        XMStoreFloat4x4(&params.worldToCameraProj, XMMatrixTranspose(worldToLightProj));
        m_Ctx->UpdateSubresource(m_CameraParamsCB, 0, nullptr, &params, sizeof(params), 0);

        //TODO: remove to scene update pass
        scene->directionalLight.worldToLight = params.worldToCamera;
        scene->directionalLight.lightToProj = params.cameraToProjection;
        scene->directionalLight.worldToLightProj = params.worldToCameraProj;
    }

    m_Ctx->OMSetRenderTargets(0, nullptr, m_DirLightShadowMapTexDSV);
    m_Ctx->ClearDepthStencilView(m_DirLightShadowMapTexDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

    D3D11_VIEWPORT viewport{};
    viewport.Width = LUMA_DIR_SHADOW_MAP_DIM;
    viewport.Height = LUMA_DIR_SHADOW_MAP_DIM;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_Ctx->RSSetViewports(1, &viewport);

    for (size_t i = 0; i < scene->meshes.size(); ++i) {
        m_Ctx->PSSetShader(nullptr, nullptr, 0);
        RenderMesh(scene, scene->meshes[i], true);
    }
}

void SceneRenderer::PointLightShadowPass(Scene *scene) noexcept {
    static const DirectX::XMVECTOR s_DirFront = DirectX::XMVectorSet(0, 0, -1, 1);
    static const DirectX::XMVECTOR s_DirBack = DirectX::XMVectorSet(0, 0, 1, 1);
    static const DirectX::XMVECTOR s_DirLeft = DirectX::XMVectorSet(-1, 0, 0, 1);
    static const DirectX::XMVECTOR s_DirRight = DirectX::XMVectorSet(1, 0, 0, 1);
    static const DirectX::XMVECTOR s_DirUp = DirectX::XMVectorSet(0, 1, 0, 1);
    static const DirectX::XMVECTOR s_DirDown = DirectX::XMVectorSet(0, -1, 0, 1);

    for (size_t lightIdx = 0; lightIdx < scene->pointLights.size(); ++lightIdx) {
        const auto light = scene->pointLights[lightIdx];

        D3D11_VIEWPORT viewport{};
        viewport.Width = LUMA_OMNIDIR_SHADOW_MAP_DIM;
        viewport.Height = LUMA_OMNIDIR_SHADOW_MAP_DIM;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        m_Ctx->RSSetViewports(1, &viewport);

        for (size_t i = 0; i < 6; ++i) {
            m_Ctx->ClearDepthStencilView(m_PointLightShadowCubemapTexarrDSVs[lightIdx * 6 + i], D3D11_CLEAR_DEPTH, 1.0f, 0);
        }

        using namespace DirectX;
        XMVECTOR pos = XMLoadFloat3(&light.position);

        XMMATRIX worldToLight[6] = {
            XMMatrixLookToLH(pos, s_DirRight, s_DirUp),
            XMMatrixLookToLH(pos, s_DirLeft, s_DirUp),
            XMMatrixLookToLH(pos, s_DirUp, s_DirFront),
            XMMatrixLookToLH(pos, s_DirDown, s_DirBack),
            XMMatrixLookToLH(pos, s_DirBack, s_DirUp),
            XMMatrixLookToLH(pos, s_DirFront, s_DirUp),
        };
        assert(std::size(worldToLight) == 6);

        for (const auto& mesh : scene->meshes) {
            HLSL::CameraParams params{};


            for (size_t i = 0; i < 6; ++i) {
                XMMATRIX worldToLightProj = XMMatrixMultiply(worldToLight[i], light.shadowmapProj);
                XMStoreFloat4x4(&params.worldToCamera, XMMatrixTranspose(worldToLight[i]));
                XMStoreFloat4x4(&params.cameraToProjection, XMMatrixTranspose(light.shadowmapProj));
                XMStoreFloat4x4(&params.worldToCameraProj, XMMatrixTranspose(worldToLightProj));
                m_Ctx->UpdateSubresource(m_CameraParamsCB, 0, nullptr, &params, sizeof(params), 0);

                m_Ctx->OMSetRenderTargets(0, nullptr, m_PointLightShadowCubemapTexarrDSVs[lightIdx * 6 + i]);

                m_Ctx->PSSetShader(g_SM.Get(PixelShaderID::CubemapShadowDepth), nullptr, 0);
                m_Ctx->PSSetConstantBuffers(0, 1, &m_CurShadowPointLightCB);

                D3D11_BOX box{};
                box.left = lightIdx * sizeof(HLSL::PointLight);
                box.right = box.left + sizeof(HLSL::PointLight);
                box.bottom = 1;
                box.back = 1;
                m_Ctx->CopySubresourceRegion(m_CurShadowPointLightCB, 0, 0, 0, 0, m_PointLights, 0, &box);
                RenderMesh(scene, mesh, true);
            }
        }
    }
}

struct MeshIACache {
    //TODO: maybe move stides, array equal elements
    UINT vaStrides[VertexAttributesEntriesCount + 1] = {};
    UINT vaOffsets[VertexAttributesEntriesCount + 1] = {};
};

void SceneRenderer::RenderMesh(const Scene* scene, const Mesh& mesh, bool depthOnly) noexcept {
    UploadMeshParams(scene, mesh);

    m_Ctx->VSSetShader(g_SM.Get(VertexShaderID::Unity), nullptr, 0);
    if (!depthOnly) {
        m_Ctx->PSSetShader(g_SM.Get(PixelShaderID::Unity), nullptr, 0);
    }

    ID3D11Buffer* vertexBuffers[VertexAttributesEntriesCount] = {};
    MeshIACache meshIA = {};

    vertexBuffers[0] = mesh.vb;
    meshIA.vaOffsets[0] = 0;
    meshIA.vaStrides[0] = mesh.vaStride;

    for (size_t i = 0; i < VertexAttributesEntriesCount; ++i) {
        auto maskItem = static_cast<VertexAttributesMask>(1 << i);
        if (bool(mesh.vaMask & maskItem)) {
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

    if (!depthOnly) {
        ID3D11Buffer* PSCBs[] = {m_CameraParamsCB, m_MeshParamsCB, m_LightParamsCB};
        m_Ctx->PSSetConstantBuffers(0, std::size(PSCBs), PSCBs);
        ID3D11ShaderResourceView* PSSRVs[] = {m_PointLightsSRV, m_DirLightShadowMapTexSRV, m_PointLightShadowCubemapTexarrSRV};
        m_Ctx->PSSetShaderResources(0, std::size(PSSRVs), PSSRVs);
        m_Ctx->PSSetSamplers(0, 1, &m_ShadowMapSMP);
    }

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
    float aspectRatio = static_cast<float>(m_TargetW) / static_cast<float>(m_TargetH);
    XMFLOAT4X4 worldToCamera = g_Cam.ViewTransform();
    XMFLOAT4X4 cameraToProjection = g_Cam.CameraToProjection(aspectRatio);

    XMMATRIX worldToCameraSIMD = XMLoadFloat4x4(&worldToCamera);
    XMMATRIX cameraToProjectionSIMD = XMLoadFloat4x4(&cameraToProjection);
    XMMATRIX worldToCameraProjSIMD = XMMatrixMultiply(worldToCameraSIMD, cameraToProjectionSIMD);

    HLSL::CameraParams params{};
    XMStoreFloat4x4(&params.worldToCamera, XMMatrixTranspose(worldToCameraSIMD));
    XMStoreFloat4x4(&params.cameraToProjection, XMMatrixTranspose(cameraToProjectionSIMD));
    XMStoreFloat4x4(&params.worldToCameraProj, XMMatrixTranspose(worldToCameraProjSIMD));

    params.worldPos = g_Cam.GetPosition();
    m_Ctx->UpdateSubresource(m_CameraParamsCB, 0, nullptr, &params, sizeof(params), 0);


    // auto a = XMMatrixInverse(nullptr, projection);
    // auto vec = XMVectorSet(0.5, 0.5, 10, 1);
    //
    // auto res = XMVector4Transform(vec, projection);
    // auto resInv = XMVector4Transform(res, a);
    // auto m = XMMatrixMultiply(projection, a);
    // auto s = 0;
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
    params.dirLight.intensity = scene->directionalLight.intensity;
    params.dirLight.worldToLightProj = scene->directionalLight.worldToLightProj;
    params.dirLight.worldToLight = scene->directionalLight.worldToLight;
    params.dirLight.lightToProj = scene->directionalLight.lightToProj;
    m_Ctx->UpdateSubresource(m_LightParamsCB, 0, nullptr, &params, sizeof(params), 0);
}

void SceneRenderer::UploadPointLights(const Scene *scene) noexcept {
    if (scene->pointLights.size() == 0)
        return;

    const size_t pointLightsCount = scene->pointLights.size();
    assert(pointLightsCount <= 128);
    HLSL::PointLight lights[128];
    for (size_t i = 0; i < scene->pointLights.size(); ++i) {
        using namespace DirectX;
        lights[i].position = scene->pointLights[i].position;
        lights[i].ambientColor = scene->pointLights[i].ambientColor;
        lights[i].diffuseColor = scene->pointLights[i].diffuseColor;
        lights[i].specularColor = scene->pointLights[i].specularColor;
        lights[i].constantAttenuation = scene->pointLights[i].constantAttenuation;
        lights[i].linearAttenuation = scene->pointLights[i].linearAttenuation;
        lights[i].quadraticAttenuation = scene->pointLights[i].quadraticAttenuation;
        XMStoreFloat4x4(&lights[i].shadowMapProjInv, XMMatrixTranspose(scene->pointLights[i].shadowmapProjInv));
        lights[i].shadowMapProjNearPlane = scene->pointLights[i].shadowMapProjNearPlane;
        lights[i].shadowMapProjFarPlane = scene->pointLights[i].shadowMapProjFarPlane;
        lights[i].shadowMapResolutionDim = LUMA_OMNIDIR_SHADOW_MAP_DIM;
    }
    m_Ctx->UpdateSubresource(m_PointLights, 0, nullptr, lights, pointLightsCount * sizeof(lights[0]), 0);
}
