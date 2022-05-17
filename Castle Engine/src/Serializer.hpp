#pragma once
#include <string>
#include <fstream>

namespace Serializer
{
	class BinaryWriter
	{
	public:
		BinaryWriter(std::string _path);
		void Close() { if(ofs.is_open()) ofs.close(); }
		template<typename T> void Serialize(const T& data);
		void WriteBytes(const void* data, int size);
	public:
		std::ofstream ofs;
	};

	class BinaryReader
	{
	public:
		BinaryReader(std::string _path);
		void Close() { if(ifs.is_open()) ifs.close(); }
		template<typename T> T Read();
		char* ReadBytes(int size);
	private:
		std::ifstream ifs;
	};

	class StringWriter
	{
	public:
		StringWriter(std::string _path);
		void Close() { if (ofs.is_open()) ofs.close(); }
		void WriteLine(const char* data);
		void WriteChar(const char value);
	public:
		std::ofstream ofs;
	};

	class StringReader
	{
	public:
		StringReader(std::string _path);
		inline void Close() { if (ifs.is_open()) ifs.close(); }
		inline char ReadChar(int size);
		inline std::string ReadLine();
	private:
		std::ifstream ifs;
	};

}