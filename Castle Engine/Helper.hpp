#pragma once
#include <type_traits>
#include <fstream>
#include "CE_Common.hpp"

#define CS_CREATE_ENUM_OPERATORS(FLAG_NAME) \
inline constexpr FLAG_NAME operator | (FLAG_NAME a, FLAG_NAME b)         \
{													           \
	using T = std::underlying_type_t<FLAG_NAME>;               \
	return (FLAG_NAME)(static_cast<T>(a) | static_cast<T>(b)); \
}															   \
inline constexpr FLAG_NAME operator & (FLAG_NAME a, FLAG_NAME b)         \
{                                                              \
	using T = std::underlying_type_t<FLAG_NAME>;               \
	return (FLAG_NAME)(static_cast<T>(a) & static_cast<T>(b)); \
}															   \
inline constexpr FLAG_NAME operator |= (FLAG_NAME a, FLAG_NAME b)        \
{                                                              \
	FLAG_NAME result = a | b;                                  \
	return result;                                             \
}															   \
inline constexpr FLAG_NAME operator &= (FLAG_NAME a, FLAG_NAME b)        \
{															   \
	FLAG_NAME result = a & b;                                  \
	return result ;											   \
}															   \
inline constexpr bool HasFlag(FLAG_NAME flag, FLAG_NAME value)	       \
{															   \
	using T = std::underlying_type_t<FLAG_NAME>;               \
	return ((T)flag & (T)value) > 0;						   \
}															   													   

void SkipBOM(std::ifstream& in);
std::string ReadAllText(const std::string& filePath);

// very fast hash function + compile time
static inline constexpr uint KnuthHash(uint a, uint shift)
{						   
	const uint knuth_hash = 2654435769u;
	return ((a * knuth_hash) >> shift);
}

// very fast hash function + compile time
static inline constexpr uint StringToHash(const char* string)
{
	uint hash = 0;
	for (uint i = 0; i < __builtin_strlen(string); ++i)
	{
		hash ^= KnuthHash(uint(string[i]), i);
	}
	return hash;
}

template <class T>
inline T* GetPackedPointer(T* p)
{
	return (T*)(((uint64)p) & 0xfffffffffffffffeULL);
}

template <class T>
inline void SetPackedPointer(T*& p, const T* p_new)
{
	uint64 packed_bit = ((uint64)p & 0x1ULL) != 0;
	p = (T*)(((uint64)p_new) | packed_bit);
}

template <class T>
inline bool GetPackedPointerBit(T* p)
{
	return ((uint64)p & 0x1ULL) != 0;
}

template <class T>
inline void SetPackedPointerBit(T*& p, bool value)
{
	uint64 u = (((uint64)p) & 0xfffffffffffffffeULL); // clear bit
	p = (T*)(u | 0x1ULL);
}

template <class T>
inline void hash_combine(std::size_t& s, const T& v)
{
	std::hash<T> h;
	s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}
