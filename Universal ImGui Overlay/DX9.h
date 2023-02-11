#ifndef __DX9__
#define __DX9__

#include <Windows.h>
#include <d3d9.h>

#include "imgui/imgui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
);

class Direct9
{
private:

	static bool show;
	
	static HWND hwnd;
	static LPCWSTR applicationName;

	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_SIZE:
            if (device != NULL && wParam != SIZE_MINIMIZED)
            {
                d3dpp.BackBufferWidth = LOWORD(lParam);
                d3dpp.BackBufferHeight = HIWORD(lParam);
                ResetDevice();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        }
        return ::DefWindowProc(hWnd, msg, wParam, lParam);
	}

	static WNDCLASSEXW wc;

	static LPDIRECT3D9 d3d9;
	static LPDIRECT3DDEVICE9 device;
	static D3DPRESENT_PARAMETERS d3dpp;


	static bool InitD3D(HWND hwnd);

	static void SetUpWindow();

    static void CleanUpDevice();
	static void ResetDevice();
public:
    static void Render();
};

#endif