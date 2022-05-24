#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

#define CGL_EXPORT __declspec(dllimport)

struct Birki
{
	int bir, iki;
};

namespace ScriptingCastle
{
	class CGL_EXPORT Program {
	public:
		static void PrintHelloCsharp();
		static Birki RefTest(Birki birki);
	};
}

namespace ScriptingEngine
{
	MonoDomain* GetDomain();
	void Initialize();
	void Destroy();
}