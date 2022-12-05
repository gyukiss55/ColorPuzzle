#pragma once

#include "framework.h"
#include <windows.h>


class FileInput {
	HANDLE       hFile;
public:
	FileInput();
	bool Open(const wchar_t * fn);
	size_t Read(void* buffer, DWORD size);
	size_t Read(std::string& str);
	void Close();
};

class FileOutput {
	HANDLE       hFile;
public:
	FileOutput();
	bool Open(const wchar_t* fn);
	size_t Write(const void* buffer, DWORD size);
	size_t Write(const std::string& str);
	void Close();
};
