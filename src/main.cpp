
#include <DirectXMath.h>
#include <d3d11.h>
#include <dxgi.h>

#include "Win/BaseWindow.h"

#include "Scene.h"
#include "SceneLoader.h"
#include "SceneRenderer.h"
#include "ShaderManager.h"

KeyID WinKeyToKeyID(WPARAM wParam) noexcept {
    switch (wParam) {
        case VK_LEFT:
            return KeyID::Left;
        case VK_RIGHT:
            return KeyID::Right;
        case VK_UP:
            return KeyID::Up;
        case VK_DOWN:
            return KeyID::Down;
        case 'Q':
            return KeyID::Q;
        case 'W':
            return KeyID::W;
        case 'E':
            return KeyID::E;
        case 'A':
            return KeyID::A;
        case 'S':
            return KeyID::S;
        case 'D':
            return KeyID::D;
        case 'Z':
            return KeyID::Z;
        case 'X':
            return KeyID::X;
        case 'C':
            return KeyID::C;
        default:
            return KeyID::None;
    }
}

class MainWindow : public BaseWindow<MainWindow>
{
public:
    LPCSTR  ClassName() const { return "MainWindow"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(m_hwnd, &ps);
                FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));
                EndPaint(m_hwnd, &ps);
                return 0;
            }
            case WM_KEYDOWN: {
                g_Input.Signal(WinKeyToKeyID(wParam), true);
                return 0;
            }
            case WM_KEYUP: {
                g_Input.Signal(WinKeyToKeyID(wParam), false);
                return 0;
            }


            default:
                return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
        }
        return TRUE;
    }
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

    HRESULT hResult = D3D11CreateDevice(selectedAdapter, D3D_DRIVER_TYPE_UNKNOWN,
                                        0, creationFlags,
                                        featureLevels, ARRAYSIZE(featureLevels),
                                        D3D11_SDK_VERSION, &device,
                                        0, &context);

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

    g_SM.Initialize(device);

    Scene scn{};
    Loader::LoadAssetsToScene(scn, "assets/beetle.obj");
    //Loader::LoadAssetsToScene(scn, "assets/stanford-bunny.obj");
//    Loader::LoadAssetsToScene(scn, "assets/cube.obj");

    {
        PointLight light{};
        light.position = {1.0f, 0.0f, 1.0f, 1.0f};
        scn.pointLights.push_back(light);

        light.position = {1.0f, 3.0f, 0.0f, 1.0f};
        scn.pointLights.push_back(light);

        light.position = {3.0f, 3.0f, 0.0f, 1.0f};
        light.diffuseColor = {0.5, 0.3, 0.8};
        light.specularColor = {0.8, 0.5, 1.0f};
        scn.pointLights.push_back(light);
    }

    Loader::UploadSceneBuffersToGPU(scn, device);

    SceneRenderer renderer{};
    renderer.Initialize(device, context);
    renderer.SetTarget(framebufferRTV, 800, 600);


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
        renderer.RenderFrame(&scn);
        swapchain->Present(1, 0);
        g_InputManager.Tick();
    }

    device->Release();
    DXGIFactory->Release();
    return 0;
}
