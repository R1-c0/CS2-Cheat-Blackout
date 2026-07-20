#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <vector>

class Memory {
public:
    Memory();
    ~Memory();
    bool FindProcess(const std::string& name);
    uintptr_t GetModuleBase(const std::string& name);
    template<typename T> T Read(uintptr_t addr) {
        T val{}; ReadProcessMemory(m_process, (LPCVOID)addr, &val, sizeof(T), nullptr); return val;
    }
    template<typename T> bool Write(uintptr_t addr, const T& val) {
        return WriteProcessMemory(m_process, (LPVOID)addr, &val, sizeof(T), nullptr);
    }
    bool ReadBuffer(uintptr_t addr, void* buf, size_t size);
    bool IsValid() { return m_process != nullptr; }
    uintptr_t client_base = 0;
    HANDLE Handle() { return m_process; }
private:
    HANDLE m_process = nullptr;
    DWORD m_pid = 0;
};
