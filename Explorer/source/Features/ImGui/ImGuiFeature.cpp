#include "ImGuiFeature.h"
#include <regex>
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
        
        // Global var exp
        DrawGlobalVariableExplorer();

        // inst var
        DrawInstanceVariableExplorer();

        // run cmd
        DrawRunCommand();

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

void ImGuiFeature::FeatureImGui::DrawInstanceVariableExplorer()
{
    ImGui::Begin("Instance Variable Explorer");

    if (ImGui::Button("Refresh"))
    {
        g_InstanceVariables = FeatureVariables::GetAllInstanceVariables();
        Interface->PrintInfo(std::to_string(g_InstanceVariables.size()));
    }

    ImGui::BeginChild("scroll_region", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
    
    for (auto instance : g_InstanceVariables) 
    {
        // Each instance
        if (ImGui::CollapsingHeader(std::to_string(instance.first).c_str()))
        {
            ImGui::BeginChild(std::format("var_scroll##{}",instance.first).c_str(), ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
            // each variable
            if (!Interface->CallBuiltin("instance_exists", {instance.first})){
                continue;
            }

            std::string valString, valPreview;
            int lbl_i = 0;
            for (const auto& var : instance.second)
            {
                valString = var.value.ToString();
                valPreview = valString.substr(0, 8) + ((valString.length() > 8) ? "..." : "");
                if (ImGui::CollapsingHeader(std::format("{} ({}): {}##{}", var.name.ToCString(), var.value.GetKindName().c_str(), valPreview, lbl_i).c_str()))
                {
                    ImGui::Text("%s", var.value.ToCString());
                }
                ++lbl_i;
            }
            ImGui::EndChild();
        }
    }


    ImGui::EndChild();
    ImGui::End();
}

void FeatureImGui::DrawRunCommand()
{
    ImGui::Begin("Run Command");

    ImGui::InputText("cmd", g_RunCommandBuffer, sizeof(g_RunCommandBuffer));

    if (ImGui::Button("Run"))
    {
        std::string cmd = g_RunCommandBuffer;

        if (FeatureImGui::RunCommand(cmd))
        {
            // Avoid duplicate of last command
            if (g_CommandHistory.empty() || std::find(g_CommandHistory.begin(), g_CommandHistory.end(), cmd) == g_CommandHistory.end())
            {
                g_CommandHistory.push_back(cmd);

                if (g_CommandHistory.size() > MAXCMDHIST)
                    g_CommandHistory.pop_front();
            }
        }
    }

    ImGui::Separator();
    ImGui::Text("Recently used:");

    for (int i = (int)g_CommandHistory.size() - 1; i >= 0; --i)
    {
        const std::string& cmd = g_CommandHistory[i];

        if (ImGui::Button(cmd.c_str()))
        {
            strncpy_s(g_RunCommandBuffer, cmd.c_str(), sizeof(g_RunCommandBuffer));
            g_RunCommandBuffer[sizeof(g_RunCommandBuffer) - 1] = '\0'; // safety
        }
    }

    ImGui::End();
}


// Private

std::vector<std::string> FeatureImGui::Tokenize(const std::string& ref)
{
    std::vector<std::string> vResults;
    size_t _beginFuncCall = ref.find_first_of('(');

    if (_beginFuncCall == std::string::npos)
    {
        return {};
    }

    size_t _endFuncCall = ref.find_first_of(')');

    if (_endFuncCall == std::string::npos)
    {
        return {};
    }

    // Function name
    vResults.push_back(ref.substr(0, _beginFuncCall));

    std::stringstream ss(ref.substr(_beginFuncCall + 1, _endFuncCall - _beginFuncCall));
    std::string sCurItem;

    while (std::getline(ss, sCurItem, ','))
    {
        sCurItem.erase(std::remove_if(sCurItem.begin(), sCurItem.end(), ::isspace), sCurItem.end());
        if (sCurItem.find_first_of(')') != std::string::npos)
        {
            auto closePos = sCurItem.find_first_of(')');

            sCurItem = sCurItem.substr(0, closePos);
        }

        if (!sCurItem.empty())
            vResults.push_back(sCurItem);
    }

    return vResults;
}

bool FeatureImGui::RunCommand(const std::string& cmd)
{
    if (cmd.empty())
        return false;

    std::string cmdcopy = cmd;

    std::regex regexAssignment(R"(global\.([a-zA-Z_]+)\s*=\s*(.*))");
    std::regex regexPeek(R"(global\.([a-zA-Z_]+))");

    if (std::regex_match(cmdcopy, regexAssignment))
    {
        cmdcopy = std::regex_replace(cmdcopy, regexAssignment, "variable_global_set(\"$1\", $2)");
    }
    else if (std::regex_match(cmdcopy, regexPeek))
    {
        cmdcopy = std::regex_replace(cmdcopy, regexPeek, "variable_global_get(\"$1\")");
    }

    std::regex validCall(R"(^[a-zA-Z_]\w*\(.*\)$)");

    if (!std::regex_match(cmdcopy, validCall))
    {
        FeatureImGui::Interface->PrintWarning("Run Command: Invalid syntax!");
        return false;
    }

    std::vector<std::string> tokens = Tokenize(cmdcopy);
    if (tokens.empty())
        return false;

    const std::string& funcName = tokens[0];

    // --- Prepare args ---
    std::vector<RValue> args(tokens.size() - 1);

    for (size_t i = 1; i < tokens.size(); i++)
    {
        const std::string& token = tokens[i];

        if (std::regex_match(token, std::regex(R"(^-?\d+(\.\d+)?$)")))
        {
            args[i - 1] = std::stod(token);
        }
        else if (std::regex_match(token, std::regex(R"REGEX("([^"]*)")REGEX")))
        {
            args[i - 1] = RValue(token.substr(1, token.size() - 2));
        }
        else if (token == "true")
        {
            args[i - 1] = true;
        }
        else if (token == "false")
        {
            args[i - 1] = false;
        }
        else
        {
            FeatureImGui::Interface->PrintWarning("Run Command: Unknown token: " + token);
            return false;
        }
    }

    RValue result;
    auto status = FeatureImGui::Interface->CallBuiltinEx(result, funcName.c_str(), nullptr, nullptr, args);
    if (!AurieSuccess(status))
    {
        FeatureImGui::Interface->PrintWarning("Run Command: Call failed: " + funcName);
        return false;
    }

    FeatureImGui::Interface->PrintInfo("Run Command: Result: %s", result.ToString());

    return true;
}