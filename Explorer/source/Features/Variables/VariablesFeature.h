#pragma once
#include <YYToolkit/YYTK_Shared.hpp>
#include "VarInfo.h"

using namespace Aurie;
using namespace YYTK;

namespace VariablesFeatures {

	class FeatureVariables
	{
	public:
		static YYTKInterface* Interface;

		static std::vector<VarInfo> GetInstanceVariables(int instanceId);
	};


}
