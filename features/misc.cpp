#include <Windows.h>
#include <thread>
#include <chrono>
#include "memory.h"
#include "sdk/offsets.h"
#include "sdk/structs.h"
#include "globals.h"
#include "features.h"

void HandleNoFlash() {
    while (true) {
        if (noFlashEnabled) {
            uintptr_t lp = g_mem.Read<uintptr_t>(g_client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
            if (lp) {
                float fd = g_mem.Read<float>(lp + 0x1424);
                if (fd > 0.f) g_mem.Write(lp + 0x1424, 0.f);
            }
        }
        Sleep(2);
    }
}

void HandleJumpThrow() {
    bool wasDown = false;
    while (true) {
        if (jumpThrowEnabled && jumpThrowKey != ImGuiKey_None) {
            bool down = GetAsyncKeyState((int)jumpThrowKey) & 0x8000;
            if (down && !wasDown) {
                INPUT mi = {}; mi.type = INPUT_MOUSE; mi.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                SendInput(1, &mi, sizeof(INPUT));
                Sleep(1000);
                INPUT ji = {}; ji.type = INPUT_KEYBOARD; ji.ki.wVk = VK_SPACE; ji.ki.dwFlags = 0;
                SendInput(1, &ji, sizeof(INPUT));
                ji.ki.dwFlags = KEYEVENTF_KEYUP; SendInput(1, &ji, sizeof(INPUT));
                Sleep(150);
                mi.mi.dwFlags = MOUSEEVENTF_LEFTUP; SendInput(1, &mi, sizeof(INPUT));
                wasDown = true;
            }
            if (!down) wasDown = false;
        }
        Sleep(1);
    }
}

void HandleFakeAngles() {
    while (true) {
        if (fakeAnglesEnabled) {
            uintptr_t lp = g_mem.Read<uintptr_t>(g_client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
            if (lp) {
                Vector3 view = g_mem.Read<Vector3>(g_client + cs2_dumper::offsets::client_dll::dwViewAngles);
                Vector3 fake = {view.x, view.y - 89.f, view.z};
                g_mem.Write(lp + 0x3340, fake);
            }
        }
        Sleep(2);
    }
}
