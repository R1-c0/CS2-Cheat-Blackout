#include <Windows.h>
#include <thread>
#include <chrono>
#include "memory.h"
#include "sdk/offsets.h"
#include "globals.h"
#include "features.h"

void HandleTriggerBot() {
    bool toggleState = false;
    bool prevKey = false;
    while (true) {
        if (triggerbotEnabled) {
            bool shouldActivate = false;
            if (triggerbotMode == 0) {
                shouldActivate = true;
            } else if (triggerbotMode == 1) {
                if (triggerbotVkCode > 0 && (GetAsyncKeyState(triggerbotVkCode) & 0x8000))
                    shouldActivate = true;
            } else if (triggerbotMode == 2) {
                bool now = triggerbotVkCode > 0 && (GetAsyncKeyState(triggerbotVkCode) & 0x8000) != 0;
                if (now && !prevKey) toggleState = !toggleState;
                prevKey = now;
                shouldActivate = toggleState;
            }
            if (shouldActivate) {
                uintptr_t lp = g_mem.Read<uintptr_t>(g_client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
                uintptr_t list = g_mem.Read<uintptr_t>(g_client + cs2_dumper::offsets::client_dll::dwEntityList);
                if (lp && list) {
                    int localTeam = g_mem.Read<int>(lp + pawn_offsets::m_iTeamNum);
                    int idx = g_mem.Read<int>(lp + pawn_offsets::m_iIDEntIndex);
                    if (idx > 0) {
                        uintptr_t entry = g_mem.Read<uintptr_t>(list + 16 + 8 * (idx >> 9));
                        if (entry) {
                            uintptr_t ent = g_mem.Read<uintptr_t>(entry + 0x70 * (idx & 0x1FF));
                            if (ent && ent != lp) {
                                int hp = g_mem.Read<int>(ent + pawn_offsets::m_iHealth);
                                int team = g_mem.Read<int>(ent + pawn_offsets::m_iTeamNum);
                                bool valid = hp > 0 && hp <= 100;
                                if (triggerbotTeamCheck) valid = valid && team != localTeam;
                                if (valid) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(2));
                                    INPUT inp = {};
                                    inp.type = INPUT_MOUSE;
                                    inp.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                                    SendInput(1, &inp, sizeof(INPUT));
                                    inp.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                                    SendInput(1, &inp, sizeof(INPUT));
                                    std::this_thread::sleep_for(std::chrono::milliseconds(triggerbotDelay));
                                }
                            }
                        }
                    }
                }
            }
        }
        Sleep(1);
    }
}
