#include "Serializer.hpp"
#include <exception>

namespace Serializer
{	
	// BINARY WRITER
	BinaryWriter::BinaryWriter(std::string _path) 
		: ofs(std::ofstream(_path, std::ios::out | std::ios::binary))
	{
		if (!ofs) throw std::exception("write file failed at oppenning!");
	}

	void BinaryWriter::WriteBytes(const void* data, int size) 
	{
		ofs.write((const char*)data, size);
	}

	template<typename T>
	void BinaryWriter::Serialize(const T& data) {
		ofs.write((char*)&data, sizeof(T));
	}

	// BINARY READER
	BinaryReader::BinaryReader(std::string _path) :
		  ifs(std::ifstream(_path, std::ios::out | std::ios::binary))
	{
		if (!ifs) throw std::exception("read file failed at oppenning!");
	}

	template<typename T>
	T BinaryReader::Read()
	{
		T value;
		ifs.read((char*)(&value),sizeof(T));
		return value;
	}

	char* BinaryReader::ReadBytes(int size)
	{
		char* result = (char*)malloc(size + 1);
		memset(result + size, 0, 1);
		ifs.read(result, size);
		return result;
	}
	// STRING WRITER
	StringWriter::StringWriter(std::string _path)
	: ofs(std::ofstream(_path, std::ios::out | std::ios::binary)) {
		if (!ofs) throw std::exception("write file failed at oppenning!");
	}
	
	void StringWriter::WriteLine(const char* data)
	{
		
	}

	void StringWriter::WriteChar(const char data)
	{
		
	}

	// STRING READER
	StringReader::StringReader(std::string _path)
	: ifs(std::ifstream(_path, std::ios::out | std::ios::binary))
	{
		if (!ifs) throw std::exception("write file failed at oppenning!");
	}
	
	inline std::string StringReader::ReadLine()
	{
		std::string line;
		std::getline(ifs, line);
		return line;
	}

	inline char StringReader::ReadChar(int size)
	{
		return 0;
	}
}