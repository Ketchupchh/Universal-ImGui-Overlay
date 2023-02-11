#include "DX9.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#pragma comment(lib, "d3d9.lib")

//Direct 9
bool Direct9::show = true;
HWND Direct9::hwnd = nullptr;
LPCWSTR Direct9::applicationName = L"App Name";    //Target application's name
WNDCLASSEXW Direct9::wc;
LPDIRECT3D9 Direct9::d3d9 = nullptr;
LPDIRECT3DDEVICE9 Direct9::device = nullptr;
D3DPRESENT_PARAMETERS Direct9::d3dpp = {};

bool Direct9::InitD3D(HWND hwnd)
{

    if ((d3d9 = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    // Create the D3DDevice
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    //d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &device) < 0)
        return false;

    return true;

}

void Direct9::CleanUpDevice()
{
    if (device != NULL)
        device->Release();

    if (d3d9 != NULL)
        d3d9->Release();

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void Direct9::ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = device->Reset(&d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

void Direct9::SetUpWindow()
{
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ImGui Overlay Universal", NULL };
    ::RegisterClassExW(&wc);

    HWND app = FindWindow(NULL, applicationName);
    while (true)
    {
        if (app == NULL)
        {
            app = FindWindow(NULL, applicationName);
        }
        else
        {
            hwnd = ::CreateWindowExW(WS_EX_LAYERED, wc.lpszClassName, L"ImGui Overlay", WS_POPUP | WS_CHILD | WS_VISIBLE, 0, 0, 1920, 1080, FindWindow(NULL, applicationName), NULL, wc.hInstance, NULL);
            ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            break;
        }
    }

    const COLORREF transparentColor = RGB(0, 0, 0);
    SetLayeredWindowAttributes(hwnd, transparentColor, 0, LWA_COLORKEY);

    if (!InitD3D(hwnd))
    {
        CleanUpDevice();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(device);
}

void Direct9::Render()
{
    SetUpWindow();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    HWND appHwnd = FindWindow(NULL, applicationName);

    DWORD game_process_id = 0;
    GetWindowThreadProcessId(appHwnd, &game_process_id);

    bool done = false;
    while (!done)
    {
        if (GetAsyncKeyState(VK_INSERT) & 1) show = !show;

        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
        if (show == true)
        {
            ImGui::CaptureMouseFromApp(true);
            ImGui::GetIO().WantCaptureMouse = true;
            ImGui::Begin("Hello, world!");

            //Your code here

            ImGui::End();

            ::SetCapture(hwnd);
            ImGui::GetIO().MouseDrawCursor = show;
        }
        else
        {
            ::ReleaseCapture();
            ::SetFocus(appHwnd);
        }

        ImGui::EndFrame();
        device->SetRenderState(D3DRS_ZENABLE, FALSE);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        if (device->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            device->EndScene();
        }
        HRESULT result = device->Present(NULL, NULL, NULL, NULL);
        if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();

        Sleep(20); //You can remove this if you want
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanUpDevice();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
}