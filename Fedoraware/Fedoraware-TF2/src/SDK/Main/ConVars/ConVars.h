#pragma once
#include "../BaseEntity/BaseEntity.h"

class CConVars
{
public:
	void Init();
	ConVar* FindVar(const char* cvarname);

private:
	std::unordered_map<FNV1A_t, ConVar*> cvarMap;
};

inline CConVars g_ConVars;