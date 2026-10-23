// Original synthetic GUI-subsystem child for Windows deployment regressions.
// Each copied executable selects a response by its own file name.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <string>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    wchar_t path[MAX_PATH]{};
    const DWORD count = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (count == 0 || count >= MAX_PATH) return 90;
    const std::wstring name(path, count);
    if (name.find(L"timeout") != std::wstring::npos) {
        Sleep(5000);
        return 91;
    }
    if (name.find(L"failure") != std::wstring::npos) {
        std::puts("production-read-only");
        return 17;
    }
    if (name.find(L"empty") != std::wstring::npos) return 0;
    if (name.find(L"unknown") != std::wstring::npos) {
        std::puts("production-read-only\nlaboratory-experimental");
        return 0;
    }
    if (name.find(L"oversized") != std::wstring::npos) {
        for (int i = 0; i < 8192; ++i) std::putchar('X');
        return 0;
    }
    if (name.find(L"stderr") != std::wstring::npos) {
        std::fputs("synthetic profile error\n", stderr);
    }
    std::puts(name.find(L"laboratory") != std::wstring::npos
        ? "laboratory-experimental" : "production-read-only");
    return 0;
}
