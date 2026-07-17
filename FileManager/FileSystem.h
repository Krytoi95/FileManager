#pragma once

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <Windows.h>
#include <string>
#include <vector>
#include <array>
#include <shlwapi.h>
#include <shellapi.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

class FileSystemEntry;
class File;
class Directory;

namespace AttrMapping {
	class AttributeMapping {
	public:
		DWORD mask;
		std::wstring name;
	};

	// Returns the file attributes as a string
	// Возвращает атрибутивы файла в строковом виде
	std::wstring getAttributesStr(const FileSystemEntry* fileSystemEntry);
}

class FileSystem {
public:
	static bool fileExist(const File& file);
	static bool directoryExist(const Directory& dir);
	static bool isOpened(const FileSystemEntry& fileSystemEntry);
	static DWORD getLastError();
	static bool createFileSystemEntry(
		FileSystemEntry* fileSystemEntry,
		const DWORD dwDesiredAccess,
		const DWORD dwCreationDisposition,
		const DWORD dwFlagsAndAttributes,
		const DWORD dwShareMode,
		const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		const HANDLE hTemplateFile
	);
	static int copyFileSystemEntry(const FileSystemEntry* originalFile, const FileSystemEntry* copyFile);
	static int moveFileSystemEntry(const FileSystemEntry* fileFrom, const FileSystemEntry* fileTo);
	static int deleteFileSystemEntry(const FileSystemEntry* fileFrom);
private:
	static SHFILEOPSTRUCTW fileOp;
};
using fs = FileSystem;

class FileSystemEntry {
public:
	FileSystemEntry(const HANDLE handle, const std::wstring& path);
	virtual ~FileSystemEntry();

	virtual bool open(
		const std::wstring& path,
		const DWORD dwDesiredAccess,
		const DWORD dwCreationDisposition,
		const DWORD dwFlagsAndAttributes,
		const DWORD dwShareMode,
		const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		const HANDLE hTemplateFile) = 0;
	virtual bool open(const std::wstring& path) = 0;
	virtual bool open() = 0;
	virtual LONGLONG getSize() = 0;
	void close();

	// Getter
	const HANDLE& getHandleConst() const;
	const std::wstring& getPathConst() const;
	const FILETIME& getFileTimeCreation() const;
	const FILETIME& getFileTimeAccess() const;
	const FILETIME& getFileTimeWrite() const;
	DWORD getDwordFlagsAndAttributes() const;

	// Setter
	void setHandle(const HANDLE& handle);

	// Updater
	bool updateInfo();
	bool updateTruePath();

protected:
	HANDLE handle {};
	std::wstring path{}; 
	LARGE_INTEGER fileSize{};
	BY_HANDLE_FILE_INFORMATION info;
};
using fse = FileSystemEntry;

class File : public FileSystemEntry {
public:
	File();
	File(const std::wstring& path);
	~File();

	// Opens the file with the required settings for the user, or the default settings if the file has not yet been created or has already been created
	// Открывает файл с нужными параметрами для пользователя или по умолчанию, если этот файл еще не был или был создан
	virtual bool open(
		const std::wstring& path,
		const DWORD dwDesiredAccess = GENERIC_READ | GENERIC_WRITE,
		const DWORD dwCreationDisposition = OPEN_ALWAYS,
		const DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL,
		const DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE,
		const LPSECURITY_ATTRIBUTES lpSecurityAttributes = nullptr,
		const HANDLE hTemplateFile = nullptr) override;

	// Opens the default file if that file has not been created or has been created
	// Открывает файл по умолчанию, если этот файл не был создан или был создан
	virtual bool open() override;

	// Opens the default file if that file has not been created or has been created
	// Открывает файл по умолчанию, если этот файл не был создан или был создан
	virtual bool open(const std::wstring& path) override;

	// Writes data to a file using the WinAPI standard
	// Записывает данные в файл по стандарту WinAPI
	bool write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped = nullptr);

	// Writes a UTF-16 string to a file
	// Записывает UTF-16 строку в файл
	bool write(const std::wstring& str, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped = nullptr);

	// Writes a UTF-8 string to a file
	// Записывает UTF-8 строку в файл
	bool write(const std::string& str, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped = nullptr);

	// Reads data from a file using the WinAPI standard
	// Читает данные из файла по стандарту WinAPI
	bool read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped = nullptr);

	// Reads a UTF-16 string from a file
	// Читает UTF-16 строку из файл
	bool read(std::wstring& str, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped = nullptr);

	// Reads a UTF-8 string from a file
	// Читает UTF-8 строку из файл
	bool read(std::string& str, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped = nullptr);

	// Getter
	const PCWSTR& getExtension() const;
	// Возвращает размер файла в Байтах, если Ошибка, возвращает -1, как код ошибки (Я просто не знаю как эту функцию переделать нормально)
	virtual LONGLONG getSize() override;
	LARGE_INTEGER getFilePointer();

	// Setter
	bool setFilePointer(LARGE_INTEGER liDistanceToMove, DWORD dwMoveMethod);

	// Updater
	bool updateExtension();

private:
	LARGE_INTEGER filePointer{};
	PCWSTR extension{};
};
using f = File;

class Directory : public FileSystemEntry {
public:
	Directory();
	Directory(const std::wstring& path);
	~Directory();

	// Opens an existing folder using the default settings or the user’s settings
	// Открывает существующую папку с параметрами по умолчанию или с параметрами пользователя
	virtual bool open(
		const std::wstring& path,
		const DWORD dwDesiredAccess = GENERIC_READ,
		const DWORD dwCreationDisposition = OPEN_EXISTING,
		const DWORD dwFlagsAndAttributes = FILE_FLAG_BACKUP_SEMANTICS,
		const DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE,
		const LPSECURITY_ATTRIBUTES lpSecurityAttributes = nullptr,
		const HANDLE hTemplateFile = nullptr) override;

	// Opens an existing folder using the default settings
	// Открывает существующую папку по умолчанию
	virtual bool open() override;

	// Opens an existing folder using the default settings
	// Открывает существубщуб папку по умолчанию
	virtual bool open(const std::wstring& path) override;
	
	virtual LONGLONG getSize() override;
};
using d = Directory;
#endif // FILESYSTEM_H
