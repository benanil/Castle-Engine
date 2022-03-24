#pragma once 

#ifndef CE_COMMON_HPP
#define CE_COMMON_HPP

#ifndef CE_INLINE 
#	ifndef _MSC_VER 
#		define CE_INLINE inline
#	else
#		define CE_INLINE __forceinline
#	endif
#endif 

typedef unsigned int uint;
typedef unsigned long uint64;

typedef unsigned char byte;

#define BitSet  (arg, posn) ((arg) |  (1L << (posn)))
#define BitClear(arg, posn) ((arg) & ~(1L << (posn)))
#define BitFlip (arg, posn) ((arg) ^  (1L << (posn)))

#endif
