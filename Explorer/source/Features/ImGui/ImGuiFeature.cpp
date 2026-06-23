#include "ImGuiFeature.h"
using namespace Aurie;
using namespace YYTK;


LRESULT CALLBACK MyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true; // ImGui handled it

    return CallWindowProc(g_OriginalWndProc, hWnd, uMsg, wParam, lParam);
}

YYTKInterface* FeatureImGui::Interface = nullptr;

void FeatureImGui::ImGuiRoutine(FWFrame& FrameContext)
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
           
        DrawGlobalVariableExplorer();

        ImGui::Render();
        g_Context->OMSetRenderTargets(1, &g_RTV, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
}


void FeatureImGui::DrawGlobalVariableExplorer()
{
    ImGui::Begin("Global Variable Explorer");

    if (ImGui::Button("Refresh"))
    {
        g_GlobalVariables = FeatureImGui::GetInstanceVariables(-5);
    }
    ImGui::BeginChild("scroll_region", ImVec2(0, 0), true);
    
    for (const auto& var : g_GlobalVariables)
    {
        ImGui::Text("%s (%s): %s", var.name.ToCString(), var.value.GetKindName().c_str(), var.value.ToCString());
    }


    ImGui::EndChild();
    ImGui::End();
}

std::vector<VarInfo> FeatureImGui::GetInstanceVariables(int instanceId)
{
    RValue instanceVarNames = Interface->CallBuiltin("variable_instance_get_names", { instanceId });
    RValue cnt = Interface->CallBuiltin("array_length", { instanceVarNames });
    std::vector<VarInfo> varNames;

    for (int i = 0; i < cnt.ToInt64(); ++i)
    {
        RValue varName = Interface->CallBuiltin("array_get", { instanceVarNames, i });
        // also get the value
        RValue varVal = Interface->CallBuiltin("variable_instance_get", { instanceId, varName });
        varNames.push_back(VarInfo(varName, varVal));
    }
    return varNames;
}
