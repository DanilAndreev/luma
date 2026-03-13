#pragma once

enum class PixelShaderID {
    Unity,
    Count,
};

enum class VertexShaderID {
    Unity,
    Count,
};

class ShaderManager {
public:
    void Initialize(ID3D11Device* device) noexcept;

public:
    ID3D11PixelShader*& Get(PixelShaderID id) noexcept;
    ID3D11VertexShader*& Get(VertexShaderID id) noexcept;
    const std::vector<char>& GetSrc(VertexShaderID id) noexcept;

private:
    std::vector<char> LoadFile(std::filesystem::path filepath) noexcept;

private:
    ID3D11PixelShader* m_PS[static_cast<size_t>(PixelShaderID::Count)] = {};
    ID3D11VertexShader* m_VS[static_cast<size_t>(VertexShaderID::Count)] = {};

    std::vector<char> m_VSSrc[static_cast<size_t>(VertexShaderID::Count)] = {};
};
