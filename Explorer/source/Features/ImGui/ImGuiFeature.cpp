#include "ImGuiFeature.h"

LRESULT CALLBACK MyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true; // ImGui handled it

    return CallWindowProc(g_OriginalWndProc, hWnd, uMsg, wParam, lParam);
}


namespace FeatureImGui {
    using namespace Aurie;
    using namespace YYTK;

    void ImGuiRoutine(FWFrame& FrameContext, YYTKInterface* Interface)
    {
        IDXGISwapChain* pSwapChain = std::get<0>(FrameContext.Arguments());
        UINT Sync = std::get<1>(FrameContext.Arguments());
        UINT Flags = std::get<2>(FrameContext.Arguments());

        if (!g_ImGuiInited)
        {
            if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_Device)))
            {
                g_Device->GetImmediateContext(&g_Context);

                DXGI_SWAP_CHAIN_DESC desc;
                pSwapChain->GetDesc(&desc);
                g_hWnd = desc.OutputWindow;

                // Hook WndProc before initializing ImGui
                g_OriginalWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)MyWndProc);

                // 
                ImGui::CreateContext();
                ImGui_ImplWin32_Init(g_hWnd);
                ImGui_ImplDX11_Init(g_Device, g_Context);

                ID3D11Texture2D* pBackBuffer = nullptr;
                pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

                if (pBackBuffer)
                {
                    g_Device->CreateRenderTargetView(pBackBuffer, NULL, &g_RTV);
                    pBackBuffer->Release();
                }

                g_ImGuiInited = true;
                Interface->PrintInfo("IMGUI READY");
            }
        }
        else // Every frame
        {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("My Overlay");

            ImGui::Text("Hello from ImGui!");

            static bool checkbox = false;
            ImGui::Checkbox("Enable Feature", &checkbox);

            static float value = 0.5f;
            ImGui::SliderFloat("Value", &value, 0.0f, 1.0f);

            if (ImGui::Button("Click Me"))
            {
                OutputDebugStringA("Button clicked!\n");
            }

            ImGui::End();

            ImGui::Render();
            g_Context->OMSetRenderTargets(1, &g_RTV, NULL);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }
    }
}