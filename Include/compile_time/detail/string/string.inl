#include "compile_time/detail/string/string.hpp"

namespace compile_time {
namespace detail {

constexpr int strcmp_diffp(detail::string a, detail::string b, std::size_t n) {
  return ((a[n] == 0) || (b[n] == 0) || (a[n] != b[n]))
             ? a[n] - b[n]
             : strcmp_diffp(a, b, n + 1);
}

}  // detail

constexpr int isalnum(int ch) {
  return isalpha(ch) || isdigit(ch);
}

constexpr int isalpha(int ch) {
  return islower(ch) || isupper(ch);
}

constexpr int islower(int ch) {
  return (ch >= 'a') && (ch <= 'z');
}

constexpr int isupper(int ch) {
  return (ch >= 'A') && (ch <= 'Z');
}

constexpr int isdigit(int ch) {
  return (ch >= '0') && (ch <= '9');
}

constexpr int isxdigit(int ch) {
  return ((ch >= 'a') && (ch <= 'f')) || ((ch >= 'A') && (ch <= 'F')) ||
         isdigit(ch);
}

constexpr int iscntrl(int ch) {
  return ((ch >= 0) && (ch <= 31)) || (ch == 127);
}

constexpr int isgraph(int ch) {
  return (ch >= 33) && (ch <= 126);
}

constexpr int isspace(int ch) {
  return (ch == '\t') || (ch == '\n') || (ch == '\v') || (ch == '\f') ||
         (ch == '\r') || (ch == ' ');
}

constexpr int isblank(int ch) {
  return (ch == '\t') || (ch == ' ');
}

constexpr int isprint(int ch) {
  return (ch >= 32) && (ch <= 126);
}

constexpr int ispunct(int ch) {
  return ((ch >= 33) && (ch <= 47)) || ((ch >= 58) && (ch <= 64)) ||
         ((ch >= 91) && (ch <= 96)) || ((ch >= 123) && (ch <= 126));
}

constexpr int tolower(int ch) {
  return isupper(ch) ? ch - 'A' + 'a' : ch;
}

constexpr int toupper(int ch) {
  return islower(ch) ? ch - 'a' + 'A' : ch;
}

constexpr std::size_t strlen(detail::string str) {
  return str.size;
}

constexpr int strcmp(detail::string a, detail::string b) {
  return (a.p == b.p) ? 0
                      : (a.size == 0 || b.size == 0)
                            ? a.size - b.size
                            : (true) ? detail::strcmp_diffp(a, b, 0) : 0;
}

}  // compile_time