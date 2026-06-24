//#include <Aurie/shared.hpp>
#include <YYToolkit/YYTK_Shared.hpp>
#include "Features/ImGui/ImGuiFeature.h"

using namespace Aurie;
using namespace YYTK;
using namespace ImGuiFeature;
using namespace VariablesFeatures;

static YYTKInterface* g_ModuleInterface = nullptr;

void FrameCallback(FWFrame& FrameContext)
{
	UNREFERENCED_PARAMETER(FrameContext);

	FeatureImGui::ImGuiRoutine(FrameContext);
}

void ExecuteItCallback(FWCodeEvent& CodeContext)
{
	UNREFERENCED_PARAMETER(CodeContext);

	CInstance* selfInst = std::get<0>(CodeContext.Arguments());
	CInstance* otherInst = std::get<1>(CodeContext.Arguments());
	CCode* codeObj = std::get<2>(CodeContext.Arguments());
	int ominousInt = std::get<3>(CodeContext.Arguments());

	//g_ModuleInterface->PrintInfo(codeObj->GetName());
	
}

EXPORTED AurieStatus ModulePreinitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus last_status = AURIE_SUCCESS;

	// Gets a handle to the interface exposed by YYTK
	// You can keep this pointer for future use, as it will not change unless YYTK is unloaded.
	g_ModuleInterface = YYTK::GetInterface();
	
	// If we can't get the interface, we fail loading.
	if (!g_ModuleInterface)
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[Explorer] Loaded!");

	FeatureImGui::Interface = g_ModuleInterface;
	FeatureVariables::Interface = g_ModuleInterface;

	last_status = g_ModuleInterface->CreateCallback(
		Module,
		EVENT_FRAME,
		FrameCallback,
		0
	);

	if (!AurieSuccess(last_status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[Explorer] - Failed to register EVENT_FRAME callback!");
	}

	// Code callback
	last_status = g_ModuleInterface->CreateCallback(
		Module,
		EVENT_OBJECT_CALL,
		ExecuteItCallback,
		0
	);

	if (!AurieSuccess(last_status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[Explorer] - Failed to register EVENT_OBJECT_CALL callback!");
	}

	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleUnload(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	return AURIE_SUCCESS;
}