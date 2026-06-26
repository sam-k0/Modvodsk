#include "VariablesFeature.h"

using namespace VariablesFeatures;

YYTKInterface* FeatureVariables::Interface = nullptr;

std::vector<VarInfo> FeatureVariables::GetInstanceVariables(int instanceId)
{
    RValue instanceVarNames = Interface->CallBuiltin("variable_instance_get_names", { instanceId });
    RValue cnt = Interface->CallBuiltin("array_length", { instanceVarNames });
    std::vector<VarInfo> vars;
    // Also get object index and spriteindex    
    vars.push_back(
        VarInfo("sprite_index", Interface->CallBuiltin("variable_instance_get", { instanceId, "sprite_index" }))
    );
    vars.push_back(
        VarInfo("object_index", Interface->CallBuiltin("variable_instance_get", { instanceId, "object_index" }))
    );

    for (int i = 0; i < cnt.ToInt64(); ++i)
    {
        RValue varName = Interface->CallBuiltin("array_get", { instanceVarNames, i });
        // also get the value
        RValue varVal = Interface->CallBuiltin("variable_instance_get", { instanceId, varName });
        vars.push_back(VarInfo(varName, varVal));
    }
    return vars;
}

std::vector<double> FeatureVariables::GetAllInstanceIds()
{
    std::vector<double> allInstIds;

    RValue instCount;
    CInstance* Global;

    Interface->GetGlobalInstance(&Global);

    Interface->GetBuiltin("instance_count", Global, NULL_INDEX, instCount);
    Interface->PrintInfo("There are currently %d instances!", instCount.ToInt64());

    for (int i = 0; i < instCount.ToInt64(); i++)
    {
        double id = Interface->CallBuiltin("instance_id_get", { (double)i }).ToDouble();
        allInstIds.push_back(id);
    }
    return allInstIds;
}

std::map<double, std::vector<VarInfo>> VariablesFeatures::FeatureVariables::GetAllInstanceVariables()
{
    std::map<double, std::vector<VarInfo>> allVars;
    auto allIds = GetAllInstanceIds();
    for (double i : allIds)
    {
        allVars.insert(std::make_pair(i, GetInstanceVariables(i)));
    }
    return allVars;
}
