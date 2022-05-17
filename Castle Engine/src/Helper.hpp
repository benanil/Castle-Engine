#pragma once
#include <type_traits>
#include <fstream>
#include "CE_Common.hpp"

void SkipBOM(std::ifstream& in);
std::string ReadAllText(const std::string& filePath);

// <summary> abcd/asd.obj
//     	         ^ </summary>
inline int GetLastBackSlashIndex(const std::string& string) {
	for (int i = string.size() - 1; i >= 0; --i)
		if (string[i] == '\\' || string[i] == '/') return i+1;
	return 0; 
}

// <summary> abcd/asd.obj
//     	  	        ^ </summary>
inline int GetExtensionIndex(const std::string& str) {
	for (int i = str.size() - 1; i >= 0; --i)
		if (str[i] == '.') return i;
	return 0;
}