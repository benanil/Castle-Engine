#pragma once

#include <cstring>



template<typename T>
inline static T DX_CREATE()
{
	T object{};
	std::memset(&object, 0, sizeof(T));

	return object;
}