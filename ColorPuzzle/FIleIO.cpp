/*
* FileIO.cpp
*/
#include "framework.h"
#include <windows.h>
#include <string>
#include "FileIO.h"

FileInput::FileInput()
{
	hFile = 0;
}
bool FileInput::Open(const wchar_t* fn)
{
	hFile = CreateFile((wchar_t*)fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	return true;
}

size_t FileInput::Read(void* buffer, DWORD size)
{
	DWORD readSize = 0;
	ReadFile(hFile, buffer, size, &readSize, NULL);
	return  readSize;
}

size_t FileInput::Read(std::string& str)
{
	DWORD lDistance = 0;
	DWORD dwPtr = SetFilePointer(hFile, lDistance, nullptr, FILE_END);
	if (dwPtr == INVALID_SET_FILE_POINTER)
		return 0;
	DWORD dwPtr0 = SetFilePointer(hFile, lDistance, nullptr, FILE_BEGIN);
	char* ptr = new char [dwPtr + 1];
	if (ptr == nullptr)
		return 0;
	DWORD readSize = 0;
	ReadFile(hFile, ptr, dwPtr, &readSize, NULL);
	if (readSize == dwPtr) {
		ptr[dwPtr] = 0;
		str = ptr;
	}
	delete ptr;
	return  readSize;
}

void FileInput::Close()
{
	CloseHandle(hFile);
	hFile = 0;
}

FileOutput::FileOutput()
{
	hFile = 0;
}

bool FileOutput::Open(const wchar_t* fn)
{
	hFile = CreateFile((wchar_t*) fn, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	return true;
}

size_t FileOutput::Write(const void* buffer, DWORD size)
{
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	BOOL bErrorFlag = FALSE;
	DWORD dwBytesWritten = 0;
	bErrorFlag = WriteFile(
		hFile,           // open file handle
		buffer,      // start of data to write
		size,  // number of bytes to write
		&dwBytesWritten, // number of bytes that were written
		NULL);            // no overlapped structure

	if (FALSE == bErrorFlag)
		return 0;
	return dwBytesWritten;
}

size_t FileOutput::Write(const std::string& str)
{
	return Write(str.c_str (), str.size ());
}

void FileOutput::Close()
{
	CloseHandle(hFile);
	hFile = 0;
}
