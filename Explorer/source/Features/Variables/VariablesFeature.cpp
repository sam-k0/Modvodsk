#include "VariablesFeature.h"

using namespace VariablesFeatures;

YYTKInterface* FeatureVariables::Interface = nullptr;

std::vector<VarInfo> FeatureVariables::GetInstanceVariables(int instanceId)
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
