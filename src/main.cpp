
#include <DirectXMath.h>
#include <d3d11.h>
#include <dxgi.h>
#include <windows.h>


#include "Scene.h"


template <class DERIVED_TYPE>
class BaseWindow
{
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        DERIVED_TYPE *pThis = NULL;

        if (uMsg == WM_NCCREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (DERIVED_TYPE*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

            pThis->m_hwnd = hwnd;
        }
        else
        {
            pThis = (DERIVED_TYPE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }
        if (pThis)
        {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }
        else
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    BaseWindow() : m_hwnd(NULL) { }

    BOOL Create(
        LPCSTR lpWindowName,
        DWORD dwStyle,
        DWORD dwExStyle = 0,
        int x = CW_USEDEFAULT,
        int y = CW_USEDEFAULT,
        int nWidth = CW_USEDEFAULT,
        int nHeight = CW_USEDEFAULT,
        HWND hWndParent = 0,
        HMENU hMenu = 0
        )
    {
        WNDCLASS wc = {0};

        wc.lpfnWndProc   = DERIVED_TYPE::WindowProc;
        wc.hInstance     = GetModuleHandle(NULL);
        wc.lpszClassName = ClassName();

        RegisterClass(&wc);

        m_hwnd = CreateWindowEx(
            dwExStyle, ClassName(), lpWindowName, dwStyle, x, y,
            nWidth, nHeight, hWndParent, hMenu, GetModuleHandle(NULL), this
            );

        return (m_hwnd ? TRUE : FALSE);
    }

    HWND Window() const { return m_hwnd; }

protected:
    virtual LPCSTR  ClassName() const = 0;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

protected:
    HWND m_hwnd;
};


class MainWindow : public BaseWindow<MainWindow>
{
public:
    LPCSTR  ClassName() const { return "MainWindow"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR pCmdLine, int nCmdShow)
{
    IDXGIFactory* DXGIFactory = nullptr;
    CreateDXGIFactory(IID_PPV_ARGS(&DXGIFactory));

    IDXGIAdapter* adapter;
    IDXGIAdapter* selectedAdapter = nullptr;
    size_t selectedAdapterVRAMSize = 0;
    for (UINT i = 0; DXGIFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC adapterDesc;
        adapter->GetDesc(&adapterDesc);
        std::wcout << i << ": " <<  adapterDesc.Description << std::endl;

        if (selectedAdapterVRAMSize < adapterDesc.DedicatedVideoMemory) {
            selectedAdapter = adapter;
            selectedAdapterVRAMSize = adapterDesc.DedicatedVideoMemory;
        }
    }

    DXGI_ADAPTER_DESC adapterDesc;
    selectedAdapter->GetDesc(&adapterDesc);
    std::wcout << "Selected adapter: " << adapterDesc.Description << std::endl;

    ID3D11Device* device;
    ID3D11DeviceContext* context;
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
// #if defined(_DEBUG)
//     creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
// #endif

    HRESULT hResult = D3D11CreateDevice(selectedAdapter, D3D_DRIVER_TYPE_UNKNOWN,
                                        0, creationFlags,
                                        featureLevels, ARRAYSIZE(featureLevels),
                                        D3D11_SDK_VERSION, &device,
                                        0, &context);

    // HRESULT hResult = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE,
    //                                 0, creationFlags,
    //                                 featureLevels, ARRAYSIZE(featureLevels),
    //                                 D3D11_SDK_VERSION, &device,
    //                                 0, &context);

    if(FAILED(hResult)){
        assert(0 && "D3D11CreateDevice() failed");
        return GetLastError();
    }

    MainWindow win;
    if (!win.Create("MyWindow", WS_OVERLAPPEDWINDOW, 0, 100, 100, 800, 600))
    {
        return 0;
    }

    ShowWindow(win.Window(), nCmdShow);

    // Create Swap Chain
    IDXGISwapChain* swapchain;
    {
        DXGI_SWAP_CHAIN_DESC swapchainDesc{};
        swapchainDesc.BufferCount = 2;
        swapchainDesc.BufferDesc.Width = 800;
        swapchainDesc.BufferDesc.Height = 600;
        swapchainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
        swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainDesc.OutputWindow = win.Window();
        swapchainDesc.SampleDesc.Count = 1;
        swapchainDesc.SampleDesc.Quality = 0;
        swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapchainDesc.Windowed = true;
        swapchainDesc.Flags = 0;

        HRESULT hResult = DXGIFactory->CreateSwapChain(device, &swapchainDesc, &swapchain);
        assert(SUCCEEDED(hResult));
    }

    ID3D11RenderTargetView* framebufferRTV;
    {
        ID3D11Texture2D* framebuffer;
        HRESULT hResult = swapchain->GetBuffer(0, IID_PPV_ARGS(&framebuffer));
        assert(SUCCEEDED(hResult));

        hResult = device->CreateRenderTargetView(framebuffer, 0, &framebufferRTV);
        assert(SUCCEEDED(hResult));
        framebuffer->Release();
    }

    // Main Loop
    bool isRunning = true;
    while(isRunning)
    {
        MSG msg = {};
        while(PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
                isRunning = false;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        FLOAT backgroundColor[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
        context->ClearRenderTargetView(framebufferRTV, backgroundColor);
        swapchain->Present(1, 0);
    }

    device->Release();
    DXGIFactory->Release();
    return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(m_hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
            EndPaint(m_hwnd, &ps);
        }
        return 0;

    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
    return TRUE;
}

struct MeshIACache {
    //TODO: maybe move stides, array equal elements
    UINT vaStrides[VertexAttributesEntriesCount] = {};
    UINT vaOffsets[VertexAttributesEntriesCount] = {};
};

void DrawMesh(Mesh* mesh) noexcept {
    ID3D11Device* m_Device;
    ID3D11DeviceContext* m_Ctx = nullptr;
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;

    m_Ctx->VSSetShader();
    m_Ctx->PSSetShader();

    ID3D11Buffer* vertexBuffers[VertexAttributesEntriesCount] = {};
    MeshIACache meshIA = {};
    size_t vaActiveIdx = 0;
    for (size_t i = 0; i < VertexAttributesEntriesCount; ++i) {
        if (mesh->vaMask & (1 << i)) {
            vertexBuffers[i] = vertexBuffer;
            meshIA.vaOffsets[i] = ResolveVAOffsetFromMask(i, mesh->vaMask, mesh->vaFlags);
        }
        meshIA.vaStrides[i] = mesh->vaStride;
    }
    m_Ctx->IASetVertexBuffers(0, VertexAttributesEntriesCount, vertexBuffers, meshIA.vaStrides, meshIA.vaOffsets);
    m_Ctx->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);


    UINT inputSlot = 0;
    D3D11_INPUT_ELEMENT_DESC inputElements[VertexAttributesEntriesCount];
    inputElements[inputSlot].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElements[inputSlot].InputSlot = inputSlot;
    inputElements[inputSlot].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    inputElements[inputSlot].InstanceDataStepRate = 0;
    inputElements[inputSlot].SemanticName = "POSITION";
    inputElements[inputSlot].SemanticIndex = 0;
    ++inputSlot;

    if (bool(mesh->vaMask & VertexAttributesMask::Normals)) {
        const auto format = bool(mesh->vaFlags & VertexAttributesFlags::HalfNormals) ? DXGI_FORMAT_R16G16_FLOAT : DXGI_FORMAT_R32G32_FLOAT;
        inputElements[inputSlot].Format = format;
        inputElements[inputSlot].InputSlot = inputSlot;
        inputElements[inputSlot].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        inputElements[inputSlot].InstanceDataStepRate = 0;
        inputElements[inputSlot].SemanticName = "NORMAL";
        inputElements[inputSlot].SemanticIndex = 0;
        ++inputSlot;
    }

    for (size_t i = 0; i < VertexAttributesMaxTexCoords; ++i) {
        if (mesh->vaMask & (VertexAttributesMask::TexCoords0 << i)) {
            const auto format = mesh->vaFlags & (VertexAttributesFlags::HalfTexCoords0 << i) ? DXGI_FORMAT_R16G16_FLOAT : DXGI_FORMAT_R32G32_FLOAT;
            inputElements[inputSlot].Format = format;
            inputElements[inputSlot].InputSlot = inputSlot;
            inputElements[inputSlot].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            inputElements[inputSlot].InstanceDataStepRate = 0;
            inputElements[inputSlot].SemanticName = "TEXCOORD";
            inputElements[inputSlot].SemanticIndex = i;
            ++inputSlot;
        }
    }

    for (size_t i = 0; i < VertexAttributesMaxColors; ++i) {
        if (mesh->vaMask & (VertexAttributesMask::Color0 << i)) {
            const auto format = mesh->vaFlags & (VertexAttributesFlags::HalfColor0 << i) ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R32G32B32A32_FLOAT;
            inputElements[inputSlot].Format = format;
            inputElements[inputSlot].InputSlot = inputSlot;
            inputElements[inputSlot].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            inputElements[inputSlot].InstanceDataStepRate = 0;
            inputElements[inputSlot].SemanticName = "COLOR";
            inputElements[inputSlot].SemanticIndex = i;
            ++inputSlot;
        }
    }
    assert(inputSlot <= 15 + 1);

    ID3D11InputLayout* inputLayout = nullptr;
    m_Device->CreateInputLayout(inputElements, inputSlot, nullptr, 0, &inputLayout);

    m_Ctx->IASetInputLayout(inputLayout);
    m_Ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}