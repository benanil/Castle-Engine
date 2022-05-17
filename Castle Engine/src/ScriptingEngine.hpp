#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

__declspec(dllexport) void PrintHelloCsharp();

namespace ScriptingEngine
{
	MonoDomain* GetDomain();
	void Initialize();
	void Destroy();
}