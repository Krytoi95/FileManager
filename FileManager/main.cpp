#include <iostream>
#include <string>

#include <io.h>    // Обязательно для _setmode
#include <fcntl.h> // Обязательно для _O_U16TEXT

#include <sstream>
#include <iomanip>

#include "FileSystem.h"

// Метод переводит FILETIME в красивую строку "ДД.ММ.ГГГГ ЧЧ:ММ:СС" (Скопировано с Gemini)
std::wstring formatFileTime(const FILETIME& fileTime) {
    FILETIME localFileTime;
    SYSTEMTIME systemTime;

    // 1. Переводим из UTC в локальное время компьютера
    if (!FileTimeToLocalFileTime(&fileTime, &localFileTime)) return L"Ошибка времени";

    // 2. Переводим в структуру SYSTEMTIME
    if (!FileTimeToSystemTime(&localFileTime, &systemTime)) return L"Ошибка времени";

    // 3. Красиво форматируем в строку через stringstream
    std::wstringstream wss;
    wss << std::setfill(L'0')
        << std::setw(2) << systemTime.wDay << L"."
        << std::setw(2) << systemTime.wMonth << L"."
        << systemTime.wYear << L" "
        << std::setw(2) << systemTime.wHour << L":"
        << std::setw(2) << systemTime.wMinute << L":"
        << std::setw(2) << systemTime.wSecond;

    return wss.str();
}


int main()
{
	_setmode(_fileno(stdout), _O_U16TEXT);
	File file(L"UTF16.txt");
	file.open();
	DWORD lpNumberOfBytesWritten{};
	LARGE_INTEGER distance{};
	distance.QuadPart = 0;
	file.setFilePointer(distance, FILE_BEGIN);
	std::wstring str{};
	DWORD bytesRead{};
	file.read(str, &bytesRead);
    std::wcout << file.getFilePointer().QuadPart << L'\n';
    std::wcout << str << L'\n' << L'\n' << L'\n' << L'\n';
    file.updateInfo();
    std::wstring timeCreation{ formatFileTime(file.getFileTimeCreation()) };
    std::wstring timeAccess{ formatFileTime(file.getFileTimeAccess()) };
    std::wstring timeWrite{ formatFileTime(file.getFileTimeWrite()) };
    std::wcout << L"Creation: " << timeCreation << L'\n';
    std::wcout << L"Access: " << timeAccess << L'\n';
    std::wcout << L"Write: " << timeWrite << L'\n';
    std::wcout << AttrMapping::getAttributesStr(file) << L'\n';
    file.updateTruePath();
    std::wcout << file.getPathConst() << L'\n';
    file.updateExtension();
    std::wcout << file.getExtension() << L'\n';
}
