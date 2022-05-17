#pragma once

#ifndef HUSTLE_CORE
#define HUSTLE_CORE

#include <iostream>
#include <type_traits>

#ifndef HS_FORCE_INLINE 
#	ifndef _MSC_VER 
#		define HS_FORCE_INLINE inline
#	else
#		define HS_FORCE_INLINE __forceinline
#	endif
#endif 

using byte = unsigned char;
using uint8 = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;
using uint64 = unsigned long;

namespace HSCore
{
	// remove flag from enum class
	template<typename Enum_T>
	inline constexpr Enum_T RemoveFlag(Enum_T& flag, Enum_T value)
	{
		using T = std::underlying_type_t<Enum_T>;
		T& result = reinterpret_cast<Enum_T&>(flag);
		result &= ~static_cast<Enum_T>(value);
		return result;
	}

	// very fast hash function + compile time
	static inline constexpr uint KnuthHash(uint a, uint shift)
	{
		const uint knuth_hash = 2654435769u;
		return ((a * knuth_hash) >> shift);
	}

	static inline constexpr void hash_combine(uint& s, const uint v, uint shift)
	{
		s ^= KnuthHash(v, shift) + 0x9e3779b9 + (s << 6) + (s >> 2);
	}

	// very fast hash function + compile time
	static inline constexpr uint StringToHash(const char* string)
	{
		uint hash = KnuthHash(string[0], 0);

		for (uint i = 1; i < __builtin_strlen(string); ++i) {
			hash_combine(hash, uint(string[i]), i);
		}

		return hash;
	}
}

#endif