#include "memory.h"
#include <tlhelp32.h>

Memory::Memory() {}
Memory::~Memory() { if (m_process) CloseHandle(m_process); }

bool Memory::FindProcess(const std::string& name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32W entry; entry.dwSize = sizeof(entry);
    int size = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
    std::vector<wchar_t> wname(size);
    MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wname.data(), size);
    std::wstring wstr(wname.data());
    bool found = false;
    if (Process32FirstW(snap, &entry)) {
        do { if (wstr == entry.szExeFile) { m_pid = entry.th32ProcessID; found = true; break; } }
        while (Process32NextW(snap, &entry));
    }
    CloseHandle(snap);
    if (found) m_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_pid);
    return m_process != nullptr;
}

uintptr_t Memory::GetModuleBase(const std::string& name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_pid);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    MODULEENTRY32W entry; entry.dwSize = sizeof(entry);
    int size = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
    std::vector<wchar_t> wname(size);
    MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wname.data(), size);
    std::wstring wstr(wname.data());
    uintptr_t base = 0;
    if (Module32FirstW(snap, &entry)) {
        do { if (wstr == entry.szModule) { base = (uintptr_t)entry.modBaseAddr; break; } }
        while (Module32NextW(snap, &entry));
    }
    CloseHandle(snap);
    return base;
}

bool Memory::ReadBuffer(uintptr_t addr, void* buf, size_t size) {
    return ReadProcessMemory(m_process, (LPCVOID)addr, buf, size, nullptr) != FALSE;
}
