#pragma once
#include "detail/string/string.hpp"

namespace compile_time {

constexpr int isalnum(int ch);
constexpr int isalpha(int ch);
constexpr int islower(int ch);
constexpr int isupper(int ch);
constexpr int isdigit(int ch);
constexpr int isxdigit(int ch);
constexpr int iscntrl(int ch);
constexpr int isgraph(int ch);
constexpr int isspace(int ch);
constexpr int isblank(int ch);
constexpr int isprint(int ch);
constexpr int ispunct(int ch);
constexpr int tolower(int ch);
constexpr int toupper(int ch);

constexpr std::size_t strlen(detail::string str);
constexpr int strcmp(detail::string a, detail::string b);
}

#include "detail/string/string.inl"