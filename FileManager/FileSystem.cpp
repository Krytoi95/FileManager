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
	std::wstring getAttributesStr(const FileSystemEntry* fileSystemEntry) {
		std::wstring result{};
		for (const AttributeMapping& attrMap : attrTable) {
			if (fileSystemEntry->getDwordFlagsAndAttributes() & attrMap.mask) {
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
bool fs::directoryExist(const Directory& dir) {
	DWORD attrs = GetFileAttributesW(dir.getPathConst().c_str());
	return (attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_DIRECTORY);
}
bool fs::isOpened(const FileSystemEntry& fileSystemEntry) {
	return fileSystemEntry.getHandleConst() != INVALID_HANDLE_VALUE;
}
DWORD fs::getLastError() { return GetLastError(); }
bool fs::createFileSystemEntry(
	FileSystemEntry* fileSystemEntry,
	const DWORD dwDesiredAccess,
	const DWORD dwCreationDisposition,
	const DWORD dwFlagsAndAttributes,
	const DWORD dwShareMode,
	const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	const HANDLE hTemplateFile
) {	
	HANDLE handle{ CreateFileW(fileSystemEntry->getPathConst().c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile) };
	if (handle == INVALID_HANDLE_VALUE) {
		return false;
	}
	fileSystemEntry->setHandle(handle);
	return true;
}
int fs::copyFileSystemEntry(const FileSystemEntry* original, const FileSystemEntry* copy) {
	std::wstring fromStr = original->getPathConst() + std::wstring(L"\0", 1);
	std::wstring toStr = copy->getPathConst() + std::wstring(L"\0", 1);
	fileOp = { 0 };
	fileOp.wFunc = FO_COPY;
	fileOp.pFrom = original->getPathConst().c_str();
	fileOp.pTo = copy->getPathConst().c_str();
	fileOp.fFlags = FOF_NOCONFIRMMKDIR;
	return SHFileOperationW(&fileOp);
}
int fs::moveFileSystemEntry(const FileSystemEntry* fileFrom, const FileSystemEntry* fileTo) {
	std::wstring fromStr = fileFrom->getPathConst() + std::wstring(L"\0", 1);
	std::wstring toStr = fileTo->getPathConst() + std::wstring(L"\0", 1);
	fileOp = { 0 };
	fileOp.wFunc = FO_MOVE;
	fileOp.pFrom = fileFrom->getPathConst().c_str();
	fileOp.pTo = fileTo->getPathConst().c_str();
	fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
	return SHFileOperationW(&fileOp);
}
int fs::deleteFileSystemEntry(const FileSystemEntry* fileFrom) {
	std::wstring fromStr = fileFrom->getPathConst() + std::wstring(L"\0", 1);
	fileOp = { 0 };
	fileOp.wFunc = FO_DELETE;
	fileOp.pFrom = fileFrom->getPathConst().c_str();
	fileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;
	return SHFileOperationW(&fileOp);
}

SHFILEOPSTRUCTW fs::fileOp { 0 };


// CLASS FileSystemEntry
fse::FileSystemEntry(const HANDLE handle, const std::wstring& path) : handle{ handle }, path{ path } {}
fse::~FileSystemEntry() { this->close(); }
void fse::close() {
	if (fs::isOpened(*this)) {
		CloseHandle(this->handle);
		this->handle = INVALID_HANDLE_VALUE;
	}
};
bool fse::updateInfo() {
	return GetFileInformationByHandle(this->handle, &info);
}
bool fse::updateTruePath() {
	DWORD bufferSize{ GetFinalPathNameByHandleW(this->handle, nullptr, 0, VOLUME_NAME_DOS) };
	if (bufferSize == 0) {
		return false;
	}
	this->path.resize(bufferSize);
	DWORD resultSize{ GetFinalPathNameByHandleW(this->handle, &path[0], bufferSize, VOLUME_NAME_DOS) };
	if (resultSize == 0 || resultSize >= bufferSize) {
		return false;
	}
	this->path.resize(resultSize);
	if (this->path.rfind(L"\\\\?\\", 0) == 0) {
		this->path = this->path.substr(4);
	}
	return true;
}
// Getter
const HANDLE& fse::getHandleConst() const { return this->handle; }
const std::wstring& fse::getPathConst() const { return this->path; }
const FILETIME& fse::getFileTimeCreation() const { return this->info.ftCreationTime; }
const FILETIME& fse::getFileTimeAccess() const { return this->info.ftLastAccessTime; }
const FILETIME& fse::getFileTimeWrite() const { return this->info.ftLastWriteTime; }
DWORD fse::getDwordFlagsAndAttributes() const { return this->info.dwFileAttributes; }

// Setter
void fse::setHandle(const HANDLE& handle) { this->handle = handle; }

// CLASS File

f::File() : FileSystemEntry(INVALID_HANDLE_VALUE, L"") {}
f::File(const std::wstring& path) : FileSystemEntry(INVALID_HANDLE_VALUE, path) {}
f::~File() {}



bool f::open(
	const std::wstring& path,
	const DWORD dwDesiredAccess,
	const DWORD dwCreationDisposition,
	const DWORD dwFlagsAndAttributes,
	const DWORD dwShareMode,
	const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	const HANDLE hTemplateFile) {
	
	this->path = path;
	fs::createFileSystemEntry(this, dwDesiredAccess, dwCreationDisposition, dwFlagsAndAttributes, dwShareMode, lpSecurityAttributes, hTemplateFile);
	return fs::isOpened(*this);
}
bool f::open() {
	return this->open(this->path, GENERIC_READ | GENERIC_WRITE, OPEN_ALWAYS);
}
bool f::open(const std::wstring& path) {
	return this->open(path, GENERIC_READ | GENERIC_WRITE, OPEN_ALWAYS);
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

bool f::updateExtension() {
	this->extension = PathFindExtensionW(this->path.c_str());
	if (*extension == L'\0') {
		return false;
	}
	return true;
}

// Getter
const PCWSTR& f::getExtension() const {
	return this->extension;
}
LONGLONG f::getSize() {
	if (!GetFileSizeEx(this->handle, &fileSize)) {
		return -1;
	}
	return this->fileSize.QuadPart;
}
LARGE_INTEGER f::getFilePointer() { return this->filePointer; }

// Setter
bool f::setFilePointer(LARGE_INTEGER liDistanceToMove, DWORD dwMoveMethod) {
	return SetFilePointerEx(this->handle, liDistanceToMove, &filePointer, dwMoveMethod);
}

// CLASS Directory

bool d::open(
	const std::wstring& path,
	const DWORD dwDesiredAccess, 
	const DWORD dwCreationDisposition,
	const DWORD dwFlagsAndAttributes,
	const DWORD dwShareMode,
	const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	const HANDLE hTemplateFile) {

	this->path = path;
	fs::createFileSystemEntry(this, dwDesiredAccess, dwCreationDisposition, dwFlagsAndAttributes, dwShareMode, lpSecurityAttributes, hTemplateFile);
	return fs::isOpened(*this);
}
bool d::open() {
	return this->open(this->path, GENERIC_READ, OPEN_EXISTING);
}

bool d::open(const std::wstring& path) {
	return this->open(path, GENERIC_READ, OPEN_EXISTING);
}

LONGLONG d::getSize() {}