/*
#include "ScriptingEngine.hpp"
#include <exception>
#include <iostream>

#ifndef BUILD_MODE_STR 
	#ifndef NDEBUG
		#define BUILD_MODE_STR "Debug"
	#else
		#define BUILD_MODE_STR "Release"
	#endif
#endif

namespace ScriptingCastle
{
	__declspec(dllexport) void Program::PrintHelloCsharp()
	{
		std::cout << "hello csharp" << std::endl;
	}

	__declspec(dllexport) Birki Program::RefTest(Birki birki)
	{
		std::cout << "C++" << birki.bir << birki.iki << std::endl;
		birki.bir += 10;
		birki.bir += 10;

		return birki;
	}
}


namespace ScriptingEngine
{
	MonoDomain* domain;
	MonoAssembly* assembly;
	MonoImage* image;

	MonoDomain* GetDomain() { return domain; }

	void Initialize()
	{
		// mono_set_dirs("x64\\" BUILD_MODE_STR, ".");
		mono_set_dirs("C:\\Users\\Administrator\\source\\repos\\Castle-Engine\\x64\\Debug", ".");

		domain = mono_jit_init("CastleEngine");

		if (!domain)  throw std::exception("mono initialization failed!");	
		
		assembly = mono_domain_assembly_open(domain,
			"C:\\Users\\Administrator\\source\\repos\\Castle-Engine\\x64\\Debug\\ScriptingCastle.dll");
		
		if (!assembly) throw std::exception("scripting engine assembly creation failed!");
		
		image = mono_assembly_get_image(assembly);

		mono_add_internal_call("ScriptingCastle.Program::PrintHelloCsharp", &ScriptingCastle::Program::PrintHelloCsharp);
		mono_add_internal_call("ScriptingCastle.Program::RefTest", &ScriptingCastle::Program::RefTest);

		MonoClass* mainClass = mono_class_from_name(image, "ScriptingCastle", "Program");
		MonoMethodDesc* methodDesc = mono_method_desc_new("ScriptingCastle.Program:Main", true);
		MonoMethodDesc* tickMethodDesc = mono_method_desc_new("ScriptingCastle.Program:Tick", true);


		MonoMethod* method     = mono_method_desc_search_in_class(methodDesc, mainClass);
		MonoMethod* tickMethod = mono_method_desc_search_in_class(tickMethodDesc, mainClass);

		MonoObject* exobj = nullptr;

		MonoObject* mainReturn = mono_runtime_invoke(method, nullptr, nullptr, &exobj);
		std::cout << "csharp main return: " << (*(int*)mono_object_unbox(mainReturn)) << std::endl;

		
	}

	void Destroy()
	{
		mono_jit_cleanup(domain);
		mono_assembly_close(assembly);
	}

}
*/
