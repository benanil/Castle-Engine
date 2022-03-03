#pragma once
#include <type_traits>
#include <fstream>

#define CS_CREATE_ENUM_OPERATORS(FLAG_NAME) \
inline FLAG_NAME operator | (FLAG_NAME a, FLAG_NAME b)         \
{													           \
	using T = std::underlying_type_t<FLAG_NAME>;               \
	return (FLAG_NAME)(static_cast<T>(a) | static_cast<T>(b)); \
}															   \
inline FLAG_NAME operator & (FLAG_NAME a, FLAG_NAME b)         \
{                                                              \
	using T = std::underlying_type_t<FLAG_NAME>;               \
	return (FLAG_NAME)(static_cast<T>(a) & static_cast<T>(b)); \
}															   \
inline FLAG_NAME operator |= (FLAG_NAME a, FLAG_NAME b)        \
{                                                              \
	FLAG_NAME result = a | b;                                  \
	return result;                                             \
}															   \
inline FLAG_NAME operator &= (FLAG_NAME a, FLAG_NAME b)        \
{															   \
	FLAG_NAME result = a & b;                                  \
	return result ;											   \
}															   \
inline bool HasFlag(FLAG_NAME flag, FLAG_NAME value)	       \
{															   \
	using T = std::underlying_type_t<FLAG_NAME>;               \
	return ((T)flag & (T)value) > 0;						   \
}															   
								
void SkipBOM(std::ifstream& in);
std::string ReadAllText(const std::string& filePath);
