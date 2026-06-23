#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include <Windows.h>
#include <d3d11.h>
#include <YYToolkit/YYTK_Shared.hpp>
#include "VarInfo.h"

using namespace Aurie;
using namespace YYTK;

// Vars
static ID3D11Device* g_Device = nullptr;
static ID3D11DeviceContext* g_Context = nullptr;
static HWND g_hWnd = nullptr;
static ID3D11RenderTargetView* g_RTV = nullptr;
static WNDPROC g_OriginalWndProc = nullptr;
// Settings
static bool g_ImGuiInited = false;
static std::vector<VarInfo> g_GlobalVariables;




extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Custom WndProc to forward messages to ImGui
LRESULT CALLBACK MyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class FeatureImGui {

public:
	static YYTKInterface* Interface;

	static void ImGuiRoutine(FWFrame& FrameContext);

	static void DrawGlobalVariableExplorer();

	static std::vector<VarInfo> GetInstanceVariables(int instanceId);
};
