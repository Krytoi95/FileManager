#pragma once

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <Windows.h>
#include <string>
#include <vector>
#include <array>

class FileSystemEntry;
class File;
class Directory;

namespace AttrMapping {
	class AttributeMapping {
	public:
		DWORD mask;
		std::wstring name;
	};

	std::wstring getAttributesStr(const File& file);
}

class FileSystem {
public:
	static bool fileExist(const File& file);
	static bool isOpened(const FileSystemEntry& fileSystemEntry);
	static void setLastError(const DWORD dwError);
	static DWORD getLastError();
	static bool createFile(
		File& file,
		const DWORD dwDesiredAccess = GENERIC_READ | GENERIC_WRITE,
		const DWORD dwCreationDisposition = OPEN_ALWAYS,
		const DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL,
		const DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE,
		const LPSECURITY_ATTRIBUTES lpSecurityAttributes = nullptr,
		const HANDLE hTemplateFile = nullptr
	);
	static bool createDirectory();
	static bool copyFile();
	static bool moveFile();
	static bool deletFile();
	static bool deleteDirectory();
};
using fs = FileSystem;

class FileSystemEntry {
public:
	FileSystemEntry(const HANDLE handle, const std::wstring& path);
	virtual ~FileSystemEntry();

	virtual bool open(const std::wstring& path) = 0;
	virtual bool open() = 0;
	virtual LONGLONG getSize() = 0;
	void close();
	bool updateInfo();
	// Getter
	const HANDLE& getHandleConst() const;
	void setHandle(const HANDLE& handle);
	const std::wstring& getPathConst() const;
	const FILETIME& getFileTimeCreation() const;
	const FILETIME& getFileTimeAccess() const;
	const FILETIME& getFileTimeWrite() const;
	DWORD getDwordFlagsAndAttributes() const;
/*	DWORD getDesiredAccess() { return this->dwDesiredAccess; }
	DWORD getShareMode() { return this->dwShareMode; }
	DWORD getFlagsAndAttributes() { return this->dwFlagsAndAttributes; }
	LPSECURITY_ATTRIBUTES getSecAttributes() { return this->lpSecurityAttributes; }
	LARGE_INTEGER getSize() { return this->size; } */


	// Setter

protected:
	HANDLE handle {};
	std::wstring path{}; 
	LARGE_INTEGER fileSize{};
	BY_HANDLE_FILE_INFORMATION info;
	/*DWORD dwDesiredAccess{}; 
	DWORD dwShareMode{};
	DWORD dwFlagsAndAttributes{}; 
	LPSECURITY_ATTRIBUTES lpSecurityAttributes{};*/
};
using fse = FileSystemEntry;

class File : public FileSystemEntry {
public:
	File();
	File(const std::wstring& path);
	~File();
	/*const LPCWSTR& lpPath, DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile*/

	// Opens the file with the required settings for the user, or the default settings if the file has not yet been created or has already been created
	// Открывает файл с нужными параметрами для пользователя или по умолчанию, если этот файл еще не был или был создан
	bool open(
		const std::wstring& path,
		const DWORD dwDesiredAccess = GENERIC_READ | GENERIC_WRITE,
		const DWORD dwCreationDisposition = OPEN_ALWAYS,
		const DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL,
		const DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE,
		const LPSECURITY_ATTRIBUTES lpSecurityAttributes = nullptr,
		const HANDLE hTemplateFile = nullptr);

	// Opens the default file if that file has not been created or has been created
	// Открывает файл по умолчанию, если этот файл не был создан или был создан
	virtual bool open() override;

	// Opens the default file if that file has not been created or has been created
	// Открывает файл по умолчанию, если этот файл не был создан или был создан
	virtual bool open(const std::wstring& path) override;

	// Возвращает размер файла в Байтах, если Ошибка, возвращает -1, как код ошибки (Я просто не знаю как эту функцию переделать нормально)
	virtual LONGLONG getSize() override;
	
	// Возвращает текущий курсор в файле
	LARGE_INTEGER getFilePointer();

	// Ставит курсор в файле
	bool setFilePointer(LARGE_INTEGER liDistanceToMove, DWORD dwMoveMethod);

	// Записывает данные в файл по стандарту WinAPI
	bool write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped = nullptr);

	// Записывает UTF-16 строку в файл
	bool write(const std::wstring& str, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped = nullptr);

	// Записывает UTF-8 строку в файл
	bool write(const std::string& str, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped = nullptr);

	// Читает данные из файла по стандарту WinAPI
	bool read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped = nullptr);

	// Читает UTF-16 строку из файл
	bool read(std::wstring& str, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped = nullptr);

	// Читает UTF-8 строку из файл
	bool read(std::string& str, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped = nullptr);

	

private:
	LARGE_INTEGER filePointer{};
};
using f = File;

class Directory : public FileSystemEntry {


};
using d = Directory;
#endif // FILESYSTEM_H
