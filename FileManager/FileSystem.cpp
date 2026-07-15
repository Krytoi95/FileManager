#include "FileSystem.h"

namespace AttrMapping {
	const std::array<AttributeMapping, 19> attrTable = { {
		{ FILE_ATTRIBUTE_DIRECTORY, L"DIRECTORY" }, // Папка
		{ FILE_ATTRIBUTE_READONLY, L"READONLY" }, // Только для чтения
		{ FILE_ATTRIBUTE_HIDDEN, L"HIDDEN" }, // Скрытый
		{ FILE_ATTRIBUTE_SYSTEM, L"SYSTEM" }, // Системный
		{ FILE_ATTRIBUTE_ARCHIVE, L"ARCHIVE" }, // Архивный
		{ FILE_ATTRIBUTE_TEMPORARY, L"TEMPORARY" }, // Временный
		{ FILE_ATTRIBUTE_SPARSE_FILE, L"SPARSE_FILE" }, // Разреженный файл
		{ FILE_ATTRIBUTE_REPARSE_POINT, L"REPARSE_POINT" }, // Точка повторной обработки
		{ FILE_ATTRIBUTE_COMPRESSED, L"COMPRESSED" }, // Сжатый
		{ FILE_ATTRIBUTE_ENCRYPTED, L"ENCRYPTED" }, // Зашифрованный
		{ FILE_ATTRIBUTE_OFFLINE, L"OFFLINE" }, // Вне сети
		{ FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, L"NOT_CONTENT_INDEXED" }, // Не индексируемый
		{ FILE_ATTRIBUTE_INTEGRITY_STREAM, L"INTEGRITY_STREAM" }, // Поток целостности
		{ FILE_ATTRIBUTE_NO_SCRUB_DATA, L"NO_SCRUB_DATA" }, // Без очистки данных
		{ FILE_ATTRIBUTE_RECALL_ON_OPEN, L"RECALL_ON_OPEN" }, // Загрузка при открытии
		{ FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS, L"RECALL_ON_DATA_ACCESS" }, // Загрузка при чтении
		{ FILE_ATTRIBUTE_PINNED, L"PINNED" }, // Всегда на устройстве
		{ FILE_ATTRIBUTE_UNPINNED, L"UNPINNED" }, // Только в облаке
		{ FILE_ATTRIBUTE_STRICTLY_SEQUENTIAL, L"STRICTLY_SEQUENTIAL" } // Последовательный доступ
	} };

	std::wstring getAttributesStr(const File& file) {
		std::wstring result{};
		for (const AttributeMapping& attrMap : attrTable) {
			if (file.getDwordFlagsAndAttributes() & attrMap.mask) {
				if (!result.empty()) {
					result += L"; ";
				}
				result += attrMap.name;
			}
		}
		
		if (result.empty()) {
			return L"Обычный файл";
		}

		return result;
	}

}

// CLASS FileSystem

bool fs::fileExist(const File& file) {
	DWORD attrs = GetFileAttributesW(file.getPathConst().c_str());
	return (attrs != INVALID_FILE_ATTRIBUTES) && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
}
bool fs::isOpened(const FileSystemEntry& fileSystemEntry) {
	return fileSystemEntry.getHandleConst() != INVALID_HANDLE_VALUE;
}
DWORD fs::getLastError() { return GetLastError(); }
bool fs::createFile(
	File& file,
	const DWORD dwDesiredAccess,
	const DWORD dwCreationDisposition,
	const DWORD dwFlagsAndAttributes,
	const DWORD dwShareMode,
	const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	const HANDLE hTemplateFile
) {
	HANDLE handle{ CreateFileW(file.getPathConst().c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile) };
	if (handle == INVALID_HANDLE_VALUE) {
		return false;
	}
	file.setHandle(handle);
	return true;
}
bool fs::createDirectory() {
	return true;
}
bool fs::copyFile() {
	return true;
}
bool fs::moveFile() {
	return true;
}
bool fs::deletFile() {
	return true;
}
bool fs::deleteDirectory() {
	return true;
}

// CLASS FileSystemEntry
fse::FileSystemEntry(const HANDLE handle, const std::wstring& path) : handle{ handle }, path{ path } {}
fse::~FileSystemEntry() { this->close(); }
//bool fse::open(const std::wstring& path) {}
//bool fse::open()
//LONGLONG fse::getSize() {}
void fse::close() {
	if (fs::isOpened(*this)) {
		CloseHandle(this->handle);
		this->handle = INVALID_HANDLE_VALUE;
	}
};
bool fse::updateInfo() {
	return GetFileInformationByHandle(this->handle, &info);
}

// Getter
const HANDLE& fse::getHandleConst() const { return this->handle; }
const std::wstring& fse::getPathConst() const { return this->path; }
const FILETIME& fse::getFileTimeCreation() const { return this->info.ftCreationTime; }
const FILETIME& fse::getFileTimeAccess() const { return this->info.ftLastAccessTime; }
const FILETIME& fse::getFileTimeWrite() const { return this->info.ftLastWriteTime; }
DWORD fse::getDwordFlagsAndAttributes() const { return this->info.dwFileAttributes; }
/*	DWORD getDesiredAccess() { return this->dwDesiredAccess; }
	DWORD getShareMode() { return this->dwShareMode; }
	DWORD getFlagsAndAttributes() { return this->dwFlagsAndAttributes; }
	LPSECURITY_ATTRIBUTES getSecAttributes() { return this->lpSecurityAttributes; }
	LARGE_INTEGER getSize() { return this->size; }
	FILETIME getTimeCreation() { return this->creation; }
	FILETIME getTimeAccess() { return this->access; }
	FILETIME getTimeWrite() { return this->write; }*/

// Setter
void fse::setHandle(const HANDLE& handle) { this->handle = handle; }

// CLASS File

f::File() : FileSystemEntry(INVALID_HANDLE_VALUE, L"") {}
f::File(const std::wstring& path) : FileSystemEntry(INVALID_HANDLE_VALUE, path) {}
f::~File() {}
/*const LPCWSTR& lpPath, DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile*/


bool f::open(
	const std::wstring& path,
	const DWORD dwDesiredAccess,
	const DWORD dwCreationDisposition,
	const DWORD dwFlagsAndAttribute,
	const DWORD dwShareMode,
	const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	const HANDLE hTemplateFile) {
	
	this->path = path;
	fs::createFile(*this, dwDesiredAccess, dwCreationDisposition, dwFlagsAndAttribute, dwShareMode, lpSecurityAttributes, hTemplateFile);
	return fs::isOpened(*this);
}
bool f::open() {
	return this->open(this->path, GENERIC_READ | GENERIC_WRITE, OPEN_ALWAYS);
}
bool f::open(const std::wstring& path) {
	return this->open(path, GENERIC_READ | GENERIC_WRITE, OPEN_ALWAYS);
}
LONGLONG f::getSize() {
	if (!GetFileSizeEx(this->handle, &fileSize)) {
		return -1;
	}
	return this->fileSize.QuadPart; 
}
LARGE_INTEGER f::getFilePointer() { return this->filePointer; }
bool f::setFilePointer(LARGE_INTEGER liDistanceToMove, DWORD dwMoveMethod) {
	return SetFilePointerEx(this->handle, liDistanceToMove, &filePointer, dwMoveMethod);
}
bool f::write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
	return WriteFile(this->handle, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}
bool f::write(const std::wstring& str, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
	DWORD nNumberOfBytesToWrite{ static_cast<DWORD>(str.size() * sizeof(wchar_t) ) };
	return write(static_cast<LPCVOID>(str.c_str()), nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}
bool f::write(const std::string& str, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
	DWORD nNumberOfBytesToWrite{ static_cast<DWORD>(str.size() * sizeof(char)) };
	return write(static_cast<LPCVOID>(str.c_str()), nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}
bool f::read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
	return ReadFile(this->handle, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}
bool f::read(std::wstring& str, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
	if (this->getSize() <= 0) {
		return false;
	}
	DWORD nNumberOfBytesToRead{ static_cast<DWORD>(this->fileSize.QuadPart) };
	size_t wcharCount{ static_cast<size_t>(nNumberOfBytesToRead) / sizeof(wchar_t) };
	str.resize(wcharCount);
	if (!this->read(str.data(), nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped)) {
		str.clear();
		return false;
	}
	str.resize(*lpNumberOfBytesRead / sizeof(wchar_t));
	return true;
}
bool f::read(std::string& str, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
	if (this->getSize() <= 0) {
		return false;
	}
	DWORD nNumberOfBytesToRead{ static_cast<DWORD>(this->fileSize.QuadPart) };
	size_t charCount{ static_cast<size_t>(nNumberOfBytesToRead) };
	str.resize(charCount);
	if (!this->read(&str[0], nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped)) {
		str.clear();
		return false;
	}
	str.resize(*lpNumberOfBytesRead);
	return true;
}

// CLASS Directory