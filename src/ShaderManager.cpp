#include "ShaderManager.h"

#include <cassert>

void ShaderManager::Initialize(ID3D11Device* device) noexcept {
    std::filesystem::path executablePath = _pgmptr;
    auto executableDir = executablePath.parent_path();
    auto compiledShadersDir = executableDir / "sm";

    {
        auto bytecode = LoadFile(compiledShadersDir / "unity.ps.dxbc");
        device->CreatePixelShader(bytecode.data(), bytecode.size(), NULL, &Get(PixelShaderID::Unity));
    }
    {
        auto bytecode = LoadFile(compiledShadersDir / "cubemapShadowDepth.ps.dxbc");
        device->CreatePixelShader(bytecode.data(), bytecode.size(), NULL, &Get(PixelShaderID::CubemapShadowDepth));
    }
    {
        const char name[] = "pointLightVisualize.ps";
        auto bytecode = LoadFile(compiledShadersDir / "pointLightVisualize.ps.dxbc");
        device->CreatePixelShader(bytecode.data(), bytecode.size(), NULL, &Get(PixelShaderID::PointLightVisualize));
        Get(PixelShaderID::PointLightVisualize)->SetPrivateData(WKPDID_D3DDebugObjectName, std::size(name), name);
    }

    {
        auto bytecode = LoadFile(compiledShadersDir / "vertex.vs.dxbc");
        device->CreateVertexShader(bytecode.data(), bytecode.size(), NULL, &Get(VertexShaderID::Unity));
        m_VSSrc[static_cast<size_t>(VertexShaderID::Unity)] = std::move(bytecode);
    }
    {
        const char name[] = "pointLightVisualize.vs";
        auto bytecode = LoadFile(compiledShadersDir / "pointLightVisualize.vs.dxbc");
        device->CreateVertexShader(bytecode.data(), bytecode.size(), NULL, &Get(VertexShaderID::PointLightVisualize));
        Get(VertexShaderID::PointLightVisualize)->SetPrivateData(WKPDID_D3DDebugObjectName, std::size(name), name);
        m_VSSrc[static_cast<size_t>(VertexShaderID::PointLightVisualize)] = std::move(bytecode);
    }

    for (size_t i = 0; i < static_cast<size_t>(PixelShaderID::Count); ++i) {
        assert(m_PS[i] != nullptr);
    }
    for (size_t i = 0; i < static_cast<size_t>(VertexShaderID::Count); ++i) {
        assert(m_VS[i] != nullptr);
    }
}

ID3D11PixelShader*& ShaderManager::Get(PixelShaderID id) noexcept {
    assert(id < PixelShaderID::Count);
    return m_PS[static_cast<size_t>(id)];
}

ID3D11VertexShader*& ShaderManager::Get(VertexShaderID id) noexcept {
    assert(id < VertexShaderID::Count);
    return m_VS[static_cast<size_t>(id)];
}

const std::vector<char> & ShaderManager::GetSrc(VertexShaderID id) noexcept {
    return m_VSSrc[static_cast<size_t>(id)];
}

std::vector<char> ShaderManager::LoadFile(std::filesystem::path filepath) noexcept {
    std::ifstream file{filepath, std::ios::binary};
    assert(file.is_open());
    file.seekg(0, std::ios_base::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios_base::beg);

    std::vector<char> bytestream{};
    bytestream.resize(size);
    file.read(bytestream.data(), size);
    assert(!file.fail() && !file.bad());
    file.close();
    return bytestream;
}
