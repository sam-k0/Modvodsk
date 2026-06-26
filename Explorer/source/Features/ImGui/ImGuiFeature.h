#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include <Windows.h>
#include <d3d11.h>
#include <deque>
#include <YYToolkit/YYTK_Shared.hpp>
#include "../Variables/VariablesFeature.h"

using namespace Aurie;
using namespace YYTK;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Custom WndProc to forward messages to ImGui
LRESULT CALLBACK MyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Maximum number of recent commands
#define MAXCMDHIST 5 


namespace ImGuiFeature {
	// Vars
	static ID3D11Device* g_Device = nullptr;
	static ID3D11DeviceContext* g_Context = nullptr;
	static HWND g_hWnd = nullptr;
	static ID3D11RenderTargetView* g_RTV = nullptr;
	static WNDPROC g_OriginalWndProc = nullptr;
	// Status vars
	static bool g_ImGuiInited = false;
	static std::vector<VarInfo> g_GlobalVariables;
	static std::deque<std::string> g_CommandHistory;

	// Input buffers
	static char g_GlobalVariableNameFilter[256] = "";
	static char g_GlobalVariableTypeNameFilter[256] = "";
	static char g_GlobalVariableValueFilter[256] = "";
	static char g_RunCommandBuffer[256] = "";

	class FeatureImGui {

	public:
		static YYTKInterface* Interface;

		static void ImGuiRoutine(FWFrame& FrameContext);

		static void DrawGlobalVariableExplorer();

		static void DrawRunCommand();

	private:
		static std::vector<std::string> Tokenize(const std::string& ref);

		static bool RunCommand(const std::string& cmd);
	};

}


