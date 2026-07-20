#include <Windows.h>
#include <cmath>
#include <thread>
#include "memory.h"
#include "sdk/offsets.h"
#include "sdk/structs.h"
#include "globals.h"
#include "features.h"

void HandleAimbot() {
    bool toggleState = false;
    bool prevKey = false;
    while (true) {
        if (aimbotEnabled) {
            bool shouldActivate = false;
            if (aimbotMode == 0) {
                shouldActivate = true;
            } else if (aimbotMode == 1) {
                if (aimbotVkCode > 0 && (GetAsyncKeyState(aimbotVkCode) & 0x8000))
                    shouldActivate = true;
            } else if (aimbotMode == 2) {
                bool now = aimbotVkCode > 0 && (GetAsyncKeyState(aimbotVkCode) & 0x8000) != 0;
                if (now && !prevKey) toggleState = !toggleState;
                prevKey = now;
                shouldActivate = toggleState;
            }
            if (shouldActivate) {
                uintptr_t lp = g_mem.Read<uintptr_t>(g_client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
                uintptr_t list = g_mem.Read<uintptr_t>(g_client + cs2_dumper::offsets::client_dll::dwEntityList);
                if (!lp || !list) { Sleep(5); continue; }
                int localHP = g_mem.Read<int>(lp + pawn_offsets::m_iHealth);
                if (localHP <= 0 || localHP > 100) { Sleep(5); continue; }
                Vector3 eye = g_mem.Read<Vector3>(lp + pawn_offsets::m_vOldOrigin); eye.z += 64.f;
                Vector3 view = g_mem.Read<Vector3>(g_client + cs2_dumper::offsets::client_dll::dwViewAngles);
                int localTeam = g_mem.Read<int>(lp + pawn_offsets::m_iTeamNum);
                float best = aimbotFov; Vector3 bestAng = {}; bool found = false;
                for (int i = 1; i <= 64; i++) {
                    uintptr_t ae = g_mem.Read<uintptr_t>(list + 16 + 8*(i>>9)); if (!ae) continue;
                    uintptr_t ac = g_mem.Read<uintptr_t>(ae + 0x70*(i&0x1FF)); if (!ac) continue;
                    uint32_t ah = g_mem.Read<uint32_t>(ac + controller_offsets::m_hPlayerPawn); if (!ah) continue;
                    uintptr_t ape = g_mem.Read<uintptr_t>(list + 16 + 8*((ah&0x7FFF)>>9)); if (!ape) continue;
                    uintptr_t ap = g_mem.Read<uintptr_t>(ape + 0x70*(ah&0x1FF)); if (!ap || ap == lp) continue;
                    int ahp = g_mem.Read<int>(ap + pawn_offsets::m_iHealth); if (ahp <= 0 || ahp > 100) continue;
                    int at = g_mem.Read<int>(ap + pawn_offsets::m_iTeamNum); if (aimbotTeamCheck && at == localTeam) continue;
                    Vector3 origin = g_mem.Read<Vector3>(ap + pawn_offsets::m_vOldOrigin);
                    Vector3 bp = origin; bp.z += aimbotHead ? 69.f : 36.f;
                    Vector3 dv = bp - eye;
                    float h = sqrtf(dv.x*dv.x + dv.y*dv.y);
                    Vector3 ang; ang.x = atan2f(-dv.z, h) * (180.f/3.14159265f);
                    ang.y = atan2f(dv.y, dv.x) * (180.f/3.14159265f); ang.z = 0;
                    if (ang.x > 89.f) ang.x = 89.f; if (ang.x < -89.f) ang.x = -89.f;
                    if (ang.y > 180.f) ang.y = 180.f; if (ang.y < -180.f) ang.y = -180.f;
                    Vector3 delta = view - ang; delta.y = fmodf(delta.y, 360.f);
                    if (delta.y > 180.f) delta.y -= 360.f; if (delta.y < -180.f) delta.y += 360.f;
                    float fov = sqrtf(delta.x*delta.x + delta.y*delta.y);
                    if (fov < best) { best = fov; bestAng = ang; found = true; }
                }
                if (found) {
                    bestAng.y += aimbotAimX;
                    bestAng.x += aimbotAimY;
                    Vector3 sm = view - bestAng; sm.y = fmodf(sm.y, 360.f);
                    if (sm.y > 180.f) sm.y -= 360.f; if (sm.y < -180.f) sm.y += 360.f;
                    Vector3 final; final.x = view.x - sm.x / aimbotSmooth;
                    final.y = view.y - sm.y / aimbotSmooth; final.z = 0;
                    if (final.x > 89.f) final.x = 89.f; if (final.x < -89.f) final.x = -89.f;
                    if (final.y > 180.f) final.y = 180.f; if (final.y < -180.f) final.y = -180.f;
                    g_mem.Write(g_client + cs2_dumper::offsets::client_dll::dwViewAngles, final);
                }
            }
        }
        Sleep(5);
    }
}
