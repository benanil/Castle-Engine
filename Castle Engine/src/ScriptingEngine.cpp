#include "ScriptingEngine.hpp"
#include <exception>
#include <iostream>

__declspec(dllexport) void PrintHelloCsharp() 
{
	std::cout << "hello csharp" << std::endl;
}

namespace ScriptingEngine
{
	MonoDomain* domain;
	MonoAssembly* assembly;
	MonoImage* image;

	MonoDomain* GetDomain() { return domain; }

	void Initialize()
	{
		mono_set_dirs("C:\\Users\\Administrator\\source\\repos\\Castle-Engine\\x64\\Debug", ".");

		domain = mono_jit_init("CastleEngine");

		if (!domain)  throw std::exception("mono initialization failed!");	
		
		assembly = mono_domain_assembly_open(domain,
		"C:\\Users\\Administrator\\source\\repos\\Castle-Engine\\x64\\Debug\\ScriptingCastle.dll");
		
		if (!assembly) throw std::exception("abariganza");
		
		image = mono_assembly_get_image(assembly);

		mono_add_internal_call("PrintHelloCsharp", &PrintHelloCsharp);

		MonoClass* mainClass = mono_class_from_name(image, "ScriptingCastle", "Program");
		MonoMethodDesc* methodDesc = mono_method_desc_new("ScriptingCastle.Program:Main", true);
		MonoMethod* method = mono_method_desc_search_in_class(methodDesc, mainClass);
		MonoObject* exobj = nullptr;
		mono_runtime_invoke(method, nullptr, nullptr, &exobj);

		
	}

	void Destroy()
	{
		mono_jit_cleanup(domain);
		mono_assembly_close(assembly);
	}

}