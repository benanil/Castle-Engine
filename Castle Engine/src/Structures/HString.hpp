#pragma once
#include "Core.hpp"
#include <iosfwd> // for overriding << operator for std::cout

namespace HS
{
	enum class StrResult
	{
		None, NotFinded = 0, IndexOutOfArray, Success
	};

	class String
	{
	public:
		~String() {
			if (ptr) { std::free(ptr); ptr = nullptr;  size = 0; capacity = 0; }
		}
		// copy constructor
		String(const String& other)
			: size(other.size),
			capacity(other.capacity),
			ptr((char*)std::calloc(capacity, sizeof(char)))
		{
			std::memcpy(ptr, other.ptr, size);
		}
		// move constructor 
		String(const String&& other) noexcept
			: size(other.size),
			capacity(other.capacity),
			ptr(other.ptr)
		{
		}

		String(int _size)
			: size(0), capacity(_size + 1),
			ptr((char*)std::calloc(capacity, sizeof(char)))
		{
		}

		String()
			: size(0), capacity(32),
			ptr((char*)std::calloc(capacity, sizeof(char)))
		{
		}

		String(const char* _ptr)
			: size(std::strlen(_ptr)),
			ptr((char*)std::calloc(capacity, sizeof(char))),
			capacity(size + 32)
		{
			std::memcpy(ptr, _ptr, size);
		}

		String(char* _ptr)
			: size(std::strlen(_ptr)),
			ptr(_ptr),//ptr((char*)std::calloc(capacity, sizeof(char))),
			capacity(size + 32)
		{
			// std::memcpy(ptr, _ptr, size);
		}
		// asign operator
		String& operator=(const String& right)
		{
			CapacityCheck(right.size);
			std::memcpy(ptr, right.ptr, right.size);
			size = right.size;
			return *this;
		}
		bool operator == (String b) { return Compare(*this, b); }
		bool operator != (String b) { return !Compare(*this, b); }
		char operator[](int index) const { return ptr[index]; }

		char* begin() { return ptr; }
		char* end() { return ptr + size; }
		const char* cbegin() const { return ptr; }
		const char* cend()   const { return ptr + size; }

		const char* c_str() const { return ptr; };

		static HS_FORCE_INLINE bool Compare(String a, String b)
		{
			return a.size == b.size && !strcmp(a.ptr, b.ptr);
		}

		void Set(const char* str)
		{
			Clear();
			CapacityCheck(strlen(str));
			std::memcpy(ptr, str, strlen(str));
		}

		void Set(const String& str)
		{
			Clear();
			CapacityCheck(str.size);
			std::memcpy(ptr, str.c_str(), str.size);
		}

		void Reserve(int size)
		{
			if (size > capacity) {
				capacity = size;
				ptr = (char*)std::realloc(ptr, capacity);
			}
		}

		void Clear() { std::memset(ptr, 0, capacity); size = 0; }

		// Append
		StrResult Insert(int index, char value)
		{
			const int findIndex = FindIndex(value);
			if (findIndex == -1) return StrResult::NotFinded;
			CapacityCheck(1);
			std::memmove(ptr + findIndex + 1, ptr + findIndex, size - findIndex);
			ptr[findIndex] = value;
			return StrResult::Success;
		}
		
		StrResult Insert(int index, const char* value)
		{
			// https://stackoverflow.com/questions/779875/what-function-is-to-replace-a-substring-from-a-string-in-c
			const int len = std::strlen(value);
			if (index > size) index = size;
			CapacityCheck(len);
			char* p = ptr + index;
			std::memmove(p + len, p, size - index);
			std::memcpy(p, value, len);
			return StrResult::Success;
		}

		StrResult Insert(int index, const String& value)
		{
			return Insert(index, value.c_str());
		}

		String& operator+= (char _char)			  { Append(_char);  return *this; }
		String& operator+= (const char* _char)	  { Append(_char);  return *this; }
		String& operator+= (const String& string) { Append(string); return *this; }

		void Append(char _char)
		{
			CapacityCheck(1);
			ptr[size++] = _char;
		}

		void Append(const char* _char)
		{
			const int len = strlen(_char);
			CapacityCheck(len);
			#pragma warning(suppress : 4996)
			std::strncat(ptr + size, _char, len + 1);
			size += len;
		}

		String AppendCopy(const char* _char)
		{
			const int len = strlen(_char);
			char* buffer = (char*)std::calloc(len + size, sizeof(char));
			std::memcpy(buffer, ptr, size);
		#pragma warning(suppress : 4996)
			std::strncat(buffer + size, _char, len + 1);
			return String(buffer);
		}

		void Append(const String& string) { Append(string.c_str()); }

		// Find
		int FindIndex(char _char) const
		{
			for (int i = 0; i < size; ++i)
				if (ptr[i] == _char) return i;
			return false;
		}

		static inline bool StrnCmp(const char* a, const char* b, int size)
		{
			return !std::strncmp(a, b, size);
		}

		int FindIndex(const char* str) const
		{
			const int len = strlen(str);
			for (int i = 0; i < size - (len-1); ++i)
				if (std::strncmp(str, ptr + i, len)) return i;
			return -1;
		}

		// Remove
		StrResult Remove(char _char)
		{
			const int index = FindIndex(_char);
			if (index == -1) return StrResult::NotFinded;
			std::memmove(ptr + index, ptr + index + 1, 1);
			--size;
			return StrResult::Success;
		}

		StrResult Remove(const char* _char)
		{
			const int len = strlen(_char);
			char* start = std::strstr(ptr, _char);
			if (start == nullptr) return StrResult::NotFinded;
			std::memset(start, 0, len);
			std::memmove(start, start + len, len);
			size -= len;
			return StrResult::Success;
		}

		StrResult Remove(const String& str)
		{
			return Remove(str.c_str());
		}

		StrResult Find(char _char) const { return FindIndex(_char) ? StrResult::Success : StrResult::NotFinded; }

		StrResult Find(const char* _char) const { return FindIndex(_char) ? StrResult::Success : StrResult::NotFinded; }

		StrResult Find(const String& str) const { return FindIndex(str.c_str()) ? StrResult::Success : StrResult::NotFinded; }

		StrResult Replace(int start, int end, const char* cstr)
		{
			if (end > capacity) return StrResult::IndexOutOfArray;
			const int len = end - start;
			std::memcpy(ptr + start, cstr, len);
			return StrResult::Success;
		}
		
		/// <summary> not suitable for big strings (1k-2k char) you can generate your own algorithm for that </summary>
		StrResult Replace(const char* old, const char* _new, int searchStartIndex = 0)
		{
			const int len = strlen(_new);
			const int oldLen = strlen(old);
			char* sp;

			if ((sp = std::strstr(ptr + searchStartIndex, old)) == nullptr) {
				return StrResult::NotFinded;
			}

			const int diff = std::abs(len - oldLen);
			const int tailLen = strlen(sp + oldLen);

			std::memmove(sp + len, sp + oldLen, tailLen + 1);
			std::memcpy(sp, _new, len);
			size += len - oldLen;

			return StrResult::Success;
		}

		/// <summary> not suitable for big strings (1k-2k char) you can generate your own algorithm for that </summary>
		StrResult ReplaceAll(const char* old, const char* _new)
		{
			bool finded = false;
			StrResult strResult = StrResult::Success;
			const int _newLen = std::strlen(_new);
			int currentIndex = 0;
			while (strResult == StrResult::Success) {
				strResult = Replace(old, _new, currentIndex);
				currentIndex += _newLen;
			}
			return finded ? StrResult::Success : StrResult::NotFinded;
		}

		StrResult Replace(const String& old, const String& _new)
		{
			return Replace(old.c_str(), _new.c_str());
		}

		String SubString(int begin, int end) const
		{
			if (begin > size) return String();
			const int buffSize = end - begin;
			char* buffer = (char*)std::calloc(buffSize + 1, 1);
			std::memcpy(buffer, ptr, buffSize);
			return String(buffer);
		}

		friend std::ostream& operator<<(std::ostream& cout, String& wstr) {
			return cout << wstr.c_str();
		}

	private:
		void CapacityCheck(int len)
		{
			if (size + len + 1 >= capacity) {
				capacity += 32 + len;
				char* old = ptr;
				ptr = (char*)std::calloc(capacity+1, 1);
				std::memcpy(ptr, old, size);
				std::free(old);
			}
		}

	private:
		int capacity;
	public:
		int size;
		char* ptr = nullptr;
	};

	class WString
	{
	public:
		~WString() {
			if (ptr) { std::free(ptr); ptr = nullptr;  size = 0; capacity = 0; }
		}
		// copy constructor
		WString(const WString& other)
			: size(other.size),
			capacity(other.capacity),
			ptr((wchar_t*)std::calloc(capacity, sizeof(wchar_t)))
		{
			std::wmemcpy(ptr, other.ptr, size);
		}
		// move constructor 
		WString(const WString&& other) noexcept
			: size(other.size),
			capacity(other.capacity),
			ptr(other.ptr)
		{
		}

		WString(int _size)
			: size(0), capacity(_size),
			ptr((wchar_t*)std::calloc(capacity, sizeof(wchar_t)))
		{
		}

		WString()
			: size(0), capacity(32),
			ptr((wchar_t*)std::calloc(capacity, sizeof(wchar_t)))
		{
		}

		WString(const wchar_t* _ptr)
			: size(std::wcslen(_ptr)),
			ptr((wchar_t*)std::calloc(capacity, sizeof(wchar_t))),
			capacity(size + 32)
		{
			std::wmemcpy(ptr, _ptr, size);
		}

		WString(wchar_t* _ptr)
			: size(std::wcslen(_ptr)),
			ptr(_ptr),//ptr((wchar_t*)std::calloc(capacity, sizeof(wchar_t))),
			capacity(size + 32)
		{
			// std::memcpy(ptr, _ptr, size * sizeof(wchar_t));
		}

		WString& operator=(const WString& right)
		{
			CapacityCheck(right.size);
			std::wmemcpy(ptr, right.ptr, right.size);
			size = right.size;
			return *this;
		}

		bool operator == (WString b) { return Compare(*this, b); }
		bool operator != (WString b) { return !Compare(*this, b); }
		wchar_t operator[](int index) const { return ptr[index]; }

		wchar_t* begin() { return ptr; }
		wchar_t* end()   { return ptr + size; }
		const wchar_t* cbegin() const { return ptr; }
		const wchar_t* cend()   const { return ptr + size; }

		const wchar_t* c_str() const { return ptr; };

		static HS_FORCE_INLINE bool Compare(WString a, WString b)
		{
			return a.size == b.size && !std::wcscmp(a.ptr, b.ptr);
		}

		void Reserve(int size)
		{
			if (size > capacity) {
				capacity = size;
				ptr = (wchar_t*)std::realloc(ptr, capacity * sizeof(wchar_t));
			}
		}

		void Clear() { std::memset(ptr, 0, size); size = 0; }

		// Append
		StrResult Insert(int index, char value)
		{
			const int findIndex = FindIndex(value);
			if (findIndex == -1) return StrResult::NotFinded;
			CapacityCheck(1);
			std::wmemmove(ptr + findIndex + 1, ptr + findIndex, 1);
			ptr[findIndex] = value;
			return StrResult::Success;
		}

		StrResult Insert(int index, const wchar_t* value)
		{
			const int len = std::wcslen(value);
			if (index > size) index = size;
			CapacityCheck(len);
			wchar_t* p = ptr + index;
			std::wmemmove(p + len, p, size - index);
			std::wmemcpy(p, value, len);
			return StrResult::Success;
		}

		StrResult Insert(int index, const WString& value)
		{
			return Insert(index, value.c_str());
		}

		WString& operator+= (char _char)		     { Append(_char);  return *this; }
		WString& operator+= (const wchar_t* _char)   { Append(_char);  return *this; }
		WString& operator+= (const WString& string)  { Append(string); return *this; }

		void Append(wchar_t _char)
		{
			CapacityCheck(1);
			ptr[size++] = _char;
		}

		void Append(const wchar_t* _char)
		{
			const int len = std::wcslen(_char);
			CapacityCheck(len);
		#pragma warning(suppress : 4996)
			std::wcsncat(ptr, _char, len+1);
			size += len;
		}

		WString AppendCopy(const wchar_t* _char) const
		{
			const int len = std::wcslen(_char);
			wchar_t* buffer = (wchar_t*)std::calloc(len + size, sizeof(wchar_t));
			std:wmemcpy(buffer, ptr, size);
		#pragma warning(suppress : 4996)
			std::wcsncat(buffer, _char, len + 1);
			return WString(buffer);
		}

		void Append(const WString& string) { Append(string.c_str()); }

		// Find
		int FindIndex(wchar_t _char) const
		{
			for (int i = 0; i < size; ++i)
				if (ptr[i] == _char) return i;
			return false;
		}

		static inline bool StrnCmp(const wchar_t* a, const wchar_t* b, int size)
		{
			return !std::wcsncmp(a, b, size);
		}

		int FindIndex(const wchar_t* str) const
		{
			const int len = std::wcslen(str);
			for (int i = 0; i < size - (len-1); ++i)
				if (!std::wcsncmp(str, ptr + i, len)) return i;
			return -1;
		}

		// Remove
		StrResult Remove(wchar_t _char)
		{
			int index = FindIndex(_char);
			if (index == 0) { Clear(); return StrResult::Success; }
			if (index == -1) return StrResult::NotFinded;
			std::wmemmove(ptr + index, ptr + index + 1, 1);
			--size;
			return StrResult::Success;
		}

		StrResult Remove(const wchar_t* _char)
		{
			const int len = std::wcslen(_char);
			wchar_t* start = std::wcsstr(ptr, _char);
			if (start == nullptr) return StrResult::NotFinded;
			std::wmemset(start, 0, len );
			std::wmemmove(start, start + len, len);
			size -= len;
			return StrResult::Success;
		}

		StrResult Remove(const WString& str)
		{
			return Remove(str.c_str());
		}

		StrResult Find(wchar_t _char) const { return FindIndex(_char) ? StrResult::Success : StrResult::NotFinded; }

		StrResult Find(const wchar_t* _char) const { return FindIndex(_char) ? StrResult::Success : StrResult::NotFinded; }

		StrResult Find(const WString& str) const { return FindIndex(str.c_str()) ? StrResult::Success : StrResult::NotFinded; }

		StrResult Replace(int start, int end, const wchar_t* cstr)
		{
			if (end > capacity) return StrResult::IndexOutOfArray;
			const int len = end - start;
			std::wmemcpy(ptr + start, cstr, len);
			return StrResult::Success;
		}

		StrResult Replace(const wchar_t* old, const wchar_t* _new, int searchStartIndex = 0)
		{
			const int len = std::wcslen(_new);
			const int oldLen = std::wcslen(old);
			wchar_t* sp;

			if ((sp = std::wcsstr(ptr + searchStartIndex, old)) == nullptr) {
				return StrResult::NotFinded;
			}

			const int diff = std::abs(len - oldLen);
			const int tailLen = std::wcslen(sp + oldLen);

			std::wmemmove(sp + len, sp + oldLen, tailLen + 1);
			std::wmemcpy(sp, _new, len);
			size += len - oldLen;

			return StrResult::Success;
		}
		/// <summary> not suitable for big strings (1k-2k char) you can generate your own algorithm for that </summary>
		StrResult ReplaceAll(const wchar_t* old, const wchar_t* _new)
		{
			bool finded = false;
			StrResult strResult = StrResult::Success;
			int currentIndex = 0;
			const int _newSize = std::wcslen(_new); 

			while (strResult == StrResult::Success) {
				strResult = Replace(old, _new, currentIndex);
				currentIndex += _newSize;
			}
			return finded ? StrResult::Success : StrResult::NotFinded;
		}

		StrResult Replace(const WString& old, const WString& _new)
		{
			return Replace(old.c_str(), _new.c_str());
		}

		String ToString() const
		{
			char* characters = (char*)std::malloc(size + 1);
		#pragma warning(suppress : 4996)
			std::wcstombs(characters, ptr, size + 1);
			return String(characters);
		}

		WString SubString(int begin, int end) const
		{
			if (begin > size) return WString();
			const int buffLen = end - begin;
			wchar_t* buffer = (wchar_t*)std::calloc(buffLen + 1, sizeof(wchar_t));
			std::wmemcpy(buffer, ptr, buffLen );
			return WString(buffer);
		}
		
		friend std::ostream& operator<<(std::ostream& cout, const WString& wstr)  {
			return cout << wstr.c_str();
		}

	private:
		void CapacityCheck(int len)
		{
			if (size + len + 1 >= capacity) {
				capacity += 32 + len;
				ptr = (wchar_t*)std::realloc(ptr, capacity * sizeof(wchar_t));
			}
		}

	private:
		int capacity;
	public:
		int size;
		wchar_t* ptr = nullptr;
	};

	static inline WString ToWString(String string)
	{
		wchar_t* buffer = (wchar_t*)std::malloc(string.size + 1 * sizeof(wchar_t));
		#pragma warning(suppress : 4996)
		std::mbstowcs(buffer, string.ptr, string.size + 1);
		return WString(buffer);
	}

	static inline wchar_t* ToWCharArray(const char* string)
	{
		const int len = std::strlen(string);
		wchar_t* buffer = (wchar_t*)std::malloc(len + 1 * sizeof(wchar_t));
		#pragma warning(suppress : 4996)
		std::mbstowcs(buffer, string, len + 1);
		return buffer;
	}

	static inline char* ToCharArray(const wchar_t* string)
	{
		const int len = std::wcslen(string);
		char* buffer = (char*)std::malloc(len + 1);
		#pragma warning(suppress : 4996)
		std::wcstombs(buffer, string, len + 1);
		return buffer;
	}
}

