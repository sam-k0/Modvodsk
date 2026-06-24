#pragma once
#include <string>
#include <YYToolkit/YYTK_Shared.hpp>

using namespace YYTK;

struct VarInfo {
	RValue name;
	RValue value;

	VarInfo(RValue name, RValue value)
	{
		this->name = name;
		this->value = value;
	}
};