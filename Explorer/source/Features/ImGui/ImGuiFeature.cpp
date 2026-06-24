#include "ImGuiFeature.h"
#include <format>
using namespace Aurie;
using namespace YYTK;
using namespace ImGuiFeature;
using namespace VariablesFeatures;

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
            Interface->PrintInfo("ImGui initialized.");
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

    ImGui::InputText("Filter Name", ImGuiFeature::g_GlobalVariableNameFilter, sizeof(ImGuiFeature::g_GlobalVariableNameFilter));
    ImGui::InputText("Filter Type", ImGuiFeature::g_GlobalVariableTypeNameFilter, sizeof(ImGuiFeature::g_GlobalVariableTypeNameFilter));
    ImGui::InputText("Filter Value", ImGuiFeature::g_GlobalVariableValueFilter, sizeof(ImGuiFeature::g_GlobalVariableValueFilter));


    if (ImGui::Button("Refresh"))
    {
        g_GlobalVariables = FeatureVariables::GetInstanceVariables(-5);
        // filter list with filters
        std::vector<VarInfo> temp;
        for (auto var : g_GlobalVariables)
        {
            if(
                var.name.ToString().contains(g_GlobalVariableNameFilter) &&
                var.value.GetKindName().contains(g_GlobalVariableTypeNameFilter) &&
                var.value.ToString().contains(g_GlobalVariableValueFilter)
            )
            {
                temp.push_back(var);
            }
        }
        g_GlobalVariables = temp;
    }
    ImGui::BeginChild("scroll_region", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

    int id = 0;
    std::string valPreview, valString;

    for (const auto& var : g_GlobalVariables)
    {
        valString = var.value.ToString();
        valPreview = valString.substr(0, 8) + ((valString.length() > 8) ? "..." : "");
        if (ImGui::CollapsingHeader(std::format("{} ({}): {}##{}", var.name.ToCString(), var.value.GetKindName().c_str(), valPreview , id).c_str()))
        {
            ImGui::Text("%s", var.value.ToCString());
        }
    }

    ImGui::EndChild();
    ImGui::End();
}

