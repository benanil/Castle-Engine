#pragma once 

#ifndef CE_COMMON_HPP
#define CE_COMMON_HPP

#include <iostream>
#include <type_traits>

#ifndef CE_INLINE 
#	ifndef _MSC_VER 
#		define CE_INLINE inline
#	else
#		define CE_INLINE __forceinline
#	endif
#endif 


// coppied from here: winnt.h line 2481  DEFINE_ENUM_FLAG_OPERATORS we are not using this because we don't want to include winnt.h
// Define operator overloads to enable bit operations on enum values that are
// used to define flags. Use HS_CREATE_ENUM_OPERATORS(YOUR_TYPE) to enable these
// Templates are defined here in order to avoid a dependency on C++ <type_traits> header file,
template <size_t S> struct _ENUM_TO_INT;
template <> struct _ENUM_TO_INT<1> { typedef char  type; };
template <> struct _ENUM_TO_INT<2> { typedef short type; };
template <> struct _ENUM_TO_INT<4> { typedef int  type; };
template <> struct _ENUM_TO_INT<8> { typedef long type; };
// used as an approximation of std::underlying_type<T>
template <class T> struct UnderlyingType {
	typedef typename _ENUM_TO_INT<sizeof(T)>::type type;
};

#define CS_CREATE_ENUM_OPERATORS(ENUMTYPE) \
inline constexpr ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) noexcept { return ENUMTYPE(((UnderlyingType<ENUMTYPE>::type)a) | ((UnderlyingType<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) noexcept { return (ENUMTYPE&)(((UnderlyingType<ENUMTYPE>::type&)a) |= ((UnderlyingType<ENUMTYPE>::type)b)); } \
inline constexpr ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) noexcept { return ENUMTYPE(((UnderlyingType<ENUMTYPE>::type)a) & ((UnderlyingType<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) noexcept { return (ENUMTYPE&)(((UnderlyingType<ENUMTYPE>::type&)a) &= ((UnderlyingType<ENUMTYPE>::type)b)); } \
inline constexpr ENUMTYPE operator ~ (ENUMTYPE a) noexcept { return ENUMTYPE(~((UnderlyingType<ENUMTYPE>::type)a)); } \
inline constexpr ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) noexcept { return ENUMTYPE(((UnderlyingType<ENUMTYPE>::type)a) ^ ((UnderlyingType<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) noexcept { return (ENUMTYPE&)(((UnderlyingType<ENUMTYPE>::type&)a) ^= ((UnderlyingType<ENUMTYPE>::type)b)); } 

#define BitSet  (arg, posn) ((arg) |  (1L << (posn)))
#define BitClear(arg, posn) ((arg) & ~(1L << (posn)))
#define BitFlip (arg, posn) ((arg) ^  (1L << (posn)))

typedef void(*Action)();

using byte	 = unsigned char;
using uint8  = unsigned char;
using ushort = unsigned short;
using uint   = unsigned int;
using uint64 = unsigned long;

template< typename T, uint N >
constexpr uint HS_ArraySize(const T(&)[N]) { return (uint)N; }

namespace CSCore
{
	// remove flag from enum class
	template<typename Enum_T>
	inline constexpr bool HasFlag(Enum_T flag, Enum_T value)
	{
		using T = std::underlying_type_t<Enum_T>;
		return static_cast<T>(flag) & static_cast<T>(value);
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
	// http://www.cse.yorku.ca/~oz/hash.html
	static inline constexpr uint StringToHash(const char* str)
	{
		uint hash = 0;
		for (int i = 0; i < __builtin_strlen(str); ++i)
			hash = str[i] + (hash << 6) + (hash << 16) - hash;
		
		return hash;
	}
}


#endif
