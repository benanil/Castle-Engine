#include "Helper.hpp"
#include "spdlog/spdlog.h"
#include <filesystem>

void SkipBOM(std::ifstream& in)
{
	char test[3] = { 0 };
	in.read(test, 3);
	if ((unsigned char)test[0] == 0xEF &&
		(unsigned char)test[1] == 0xBB &&
		(unsigned char)test[2] == 0xBF) {
		return;
	}
	in.seekg(0);
}

std::string ReadAllText(const std::string& filePath)
{
	if (!std::filesystem::exists(filePath)) {
		spdlog::warn("file is not exist! {0} ", filePath);
	}

	std::ifstream f(filePath, std::ios::in | std::ios::binary | std::ios::failbit);
	SkipBOM(f);
	const auto sz = std::filesystem::file_size(filePath);
	std::string result(sz, '\0');
	f.read(result.data(), sz);
	return result;
}

