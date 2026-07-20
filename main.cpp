#include <Windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <cstdint>
#include <cstdio>
#include <chrono>
#include <thread>
#include <string>
#include <cmath>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_dx11.h"
#include "imgui/backends/imgui_impl_win32.h"

#include "memory.h"
#include "sdk/offsets.h"
#include "sdk/structs.h"
#include "cs/bone.hpp"
#include "cs/weapon_index.h"
#include "render.h"
#include "globals.h"
#include "features/features.h"

#pragma comment(lib, "dwmapi.lib")

static ImVec4 bgColor           = ImVec4(25/255.f, 25/255.f, 25/255.f, 0.95f);
static ImVec4 tabActiveColor    = ImVec4(60/255.f, 60/255.f, 60/255.f, 1.00f);
static ImVec4 tabInactiveColor  = ImVec4(15/255.f, 15/255.f, 15/255.f, 1.00f);
static ImVec4 tabHoveredColor   = ImVec4(45/255.f, 45/255.f, 45/255.f, 1.00f);
static ImVec4 watermarkText     = ImVec4(180/255.f,180/255.f,180/255.f, 1.00f);
static ImVec4 watermarkBg       = ImVec4(20/255.f, 20/255.f, 20/255.f, 0.50f);

static HWND FindCS2() { return FindWindowA("Valve001", nullptr); }

static bool WorldToScreen(const Vector3& wp, Vector2& sp, const view_matrix_t& vm, int sw, int sh) {
    float w = vm.m[3][0]*wp.x + vm.m[3][1]*wp.y + vm.m[3][2]*wp.z + vm.m[3][3];
    if (w < 0.001f) return false;
    float x = vm.m[0][0]*wp.x + vm.m[0][1]*wp.y + vm.m[0][2]*wp.z + vm.m[0][3];
    float y = vm.m[1][0]*wp.x + vm.m[1][1]*wp.y + vm.m[1][2]*wp.z + vm.m[1][3];
    sp.x = (sw/2.f)*(1.f + x/w); sp.y = (sh/2.f)*(1.f - y/w);
    return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    if (msg == WM_DESTROY) PostQuitMessage(0);
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

static void ToggleTransparency(HWND hwnd, bool transparent) {
    LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (transparent) style |= WS_EX_TRANSPARENT;
    else             style &= ~WS_EX_TRANSPARENT;
    SetWindowLong(hwnd, GWL_EXSTYLE, style);
}

const char* GetWeaponName(uint64_t csPlayerPawn, uintptr_t entityList) {
    static char buf[64]; buf[0] = '\0';
    uintptr_t ws = g_mem.Read<uintptr_t>(csPlayerPawn + pawn_offsets::m_pWeaponServices);
    if (!ws) { strcpy_s(buf, "?"); return buf; }
    uint32_t handle = g_mem.Read<uint32_t>(ws + weapon_offsets::m_hActiveWeapon);
    int index = handle & 0x7FFF;
    if (!index) { strcpy_s(buf, "?"); return buf; }
    uintptr_t entry = g_mem.Read<uintptr_t>(entityList + 16 + 8 * (index >> 9));
    if (!entry) { strcpy_s(buf, "?"); return buf; }
    uintptr_t weapon = g_mem.Read<uintptr_t>(entry + 0x70 * (index & 0x1FF));
    if (!weapon) { strcpy_s(buf, "?"); return buf; }
    uint16_t idx = g_mem.Read<uint16_t>(weapon + weapon_offsets::m_AttributeManager + weapon_offsets::m_Item + weapon_offsets::m_iItemDefinitionIndex);
    for (const auto& w : IndexToWeapon)
        if (w.first == idx) { strncpy_s(buf, w.second.c_str(), sizeof(buf)-1); return buf; }
    strcpy_s(buf, "Unknown"); return buf;
}

int main() {
    SetConsoleTitleA("Black0ut CS2");

    printf("[+] Waiting for cs2.exe...\n");
    while (!g_mem.FindProcess("cs2.exe")) Sleep(1000);
    printf("[+] cs2.exe found (PID: %d)\n", GetProcessId(g_mem.Handle()));

    while (!g_client) { g_client = g_mem.GetModuleBase("client.dll"); Sleep(100); }
    printf("[+] client.dll at 0x%llX\n", g_client);
    g_mem.client_base = g_client;

    printf("[+] Creating overlay window...\n");
    sw = GetSystemMetrics(SM_CXSCREEN); sh = GetSystemMetrics(SM_CYSCREEN);
    WNDCLASSEXW wc = {}; wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"Black0utOverlay";
    RegisterClassExW(&wc);

    HWND overlay = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        wc.lpszClassName, L"Black0ut", WS_POPUP, 0, 0, sw, sh,
        nullptr, nullptr, wc.hInstance, nullptr);
    if (!overlay) { printf("[-] Overlay window creation failed\n"); return 1; }

    SetLayeredWindowAttributes(overlay, 0, 255, LWA_ALPHA);
    MARGINS margins = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(overlay, &margins);
    ShowWindow(overlay, SW_SHOW);
    printf("[+] Overlay window created\n");

    printf("[+] Creating D3D11 device + swap chain...\n");
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.Width = 0; sd.BufferDesc.Height = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = overlay;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_11_0;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* ctx = nullptr;
    IDXGISwapChain* swap = nullptr;
    ID3D11RenderTargetView* target = nullptr;
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        &fl, 1, D3D11_SDK_VERSION, &sd, &swap, &device, nullptr, &ctx))) {
        printf("[-] D3D11 creation failed\n"); return 1;
    }
    ID3D11Texture2D* back = nullptr;
    swap->GetBuffer(0, IID_PPV_ARGS(&back));
    if (back) { device->CreateRenderTargetView(back, nullptr, &target); back->Release(); }
    if (!target) { printf("[-] Render target view creation failed\n"); return 1; }
    printf("[+] D3D11 ready\n");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(overlay);
    ImGui_ImplDX11_Init(device, ctx);

    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = 0; s.FrameRounding = 0; s.ScrollbarRounding = 0; s.GrabRounding = 0;
    s.ChildRounding = 0; s.PopupRounding = 0; s.TabRounding = 0;
    s.WindowBorderSize = 1; s.FrameBorderSize = 0; s.PopupBorderSize = 0;
    s.FramePadding = ImVec2(10, 6); s.ItemSpacing = ImVec2(8, 6);
    s.WindowPadding = ImVec2(12, 10); s.ScrollbarSize = 12;
    s.Colors[ImGuiCol_WindowBg]        = bgColor;
    s.Colors[ImGuiCol_TabActive]       = tabActiveColor;
    s.Colors[ImGuiCol_Tab]             = tabInactiveColor;
    s.Colors[ImGuiCol_TabHovered]      = tabHoveredColor;
    s.Colors[ImGuiCol_Text]            = ImVec4(220/255.f,220/255.f,220/255.f, 1.00f);
    s.Colors[ImGuiCol_CheckMark]       = ImVec4(100/255.f,180/255.f,255/255.f, 1.00f);
    s.Colors[ImGuiCol_FrameBg]         = ImVec4(35/255.f, 35/255.f, 35/255.f, 1.00f);
    s.Colors[ImGuiCol_FrameBgHovered]  = ImVec4(50/255.f, 50/255.f, 50/255.f, 1.00f);
    s.Colors[ImGuiCol_FrameBgActive]   = ImVec4(50/255.f, 50/255.f, 50/255.f, 1.00f);
    s.Colors[ImGuiCol_Button]          = ImVec4(55/255.f, 55/255.f, 55/255.f, 1.00f);
    s.Colors[ImGuiCol_ButtonHovered]   = ImVec4(70/255.f,70/255.f,70/255.f,1);
    s.Colors[ImGuiCol_ButtonActive]    = ImVec4(40/255.f,40/255.f,40/255.f,1);
    s.Colors[ImGuiCol_Border]          = ImVec4(45/255.f,45/255.f,45/255.f,1);
    s.Colors[ImGuiCol_Header]          = tabActiveColor;
    s.Colors[ImGuiCol_HeaderHovered]   = tabHoveredColor;
    s.Colors[ImGuiCol_SliderGrab]      = ImVec4(100/255.f,180/255.f,255/255.f, 1.00f);
    s.Colors[ImGuiCol_SliderGrabActive]= ImVec4(130/255.f,200/255.f,255/255.f,1);

    RefreshConfigs();
    LoadConfig();

    std::thread(HandleNoFlash).detach();
    std::thread(HandleJumpThrow).detach();
    std::thread(HandleFakeAngles).detach();
    std::thread(HandleTriggerBot).detach();
    std::thread(HandleAimbot).detach();

    int fps = 0, frameCount = 0;
    auto lastTime = std::chrono::steady_clock::now();
    bool running = true;
    MSG msg = {};

    while (running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg); DispatchMessage(&msg);
            if (msg.message == WM_QUIT) running = false;
        }

        if ((GetAsyncKeyState(VK_INSERT) & 1) || (GetAsyncKeyState(VK_DELETE) & 1)) {
            guiActive = !guiActive;
            io.MouseDrawCursor = guiActive;
            ToggleTransparency(overlay, !guiActive);
        }

        frameCount++;
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration<float>(now - lastTime).count() >= 1.f) {
            fps = frameCount; frameCount = 0; lastTime = now;
        }

        HWND csgo = FindCS2();
        if (csgo) {
            RECT r; GetClientRect(csgo, &r);
            POINT ul = {0,0}; ClientToScreen(csgo, &ul);
            SetWindowPos(overlay, HWND_TOPMOST, ul.x, ul.y, r.right, r.bottom, SWP_SHOWWINDOW);
            sw = r.right; sh = r.bottom;
        }

        uintptr_t localPawn = g_mem.Read<uintptr_t>(g_client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
        uintptr_t localCtrl = g_mem.Read<uintptr_t>(g_client + cs2_dumper::offsets::client_dll::dwLocalPlayerController);
        view_matrix_t vm    = g_mem.Read<view_matrix_t>(g_client + cs2_dumper::offsets::client_dll::dwViewMatrix);
        uintptr_t list      = g_mem.Read<uintptr_t>(g_client + cs2_dumper::offsets::client_dll::dwEntityList);

        int localTeam = 0, localHP = 0, ping = 0;
        if (localPawn) { localTeam = g_mem.Read<int>(localPawn + pawn_offsets::m_iTeamNum);
                         localHP   = g_mem.Read<int>(localPawn + pawn_offsets::m_iHealth); }
        if (localCtrl) ping = g_mem.Read<int>(localCtrl + 0x64C);

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (guiActive) {
            ImGui::SetNextWindowSize(ImVec2(680, 480), ImGuiCond_Once);
            ImGui::Begin("Black0ut", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

            {
                ImVec2 wp = ImGui::GetWindowPos(), ws = ImGui::GetWindowSize();
                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(ImVec2(wp.x,wp.y), ImVec2(wp.x+ws.x,wp.y+32), IM_COL32(35,35,35,255));
                dl->AddText(ImVec2(wp.x+14,wp.y+7), IM_COL32(200,200,200,255), "Black0ut");
            }

            if (ImGui::BeginTabBar("Tabs", ImGuiTabBarFlags_NoTabListScrollingButtons)) {
                if (ImGui::BeginTabItem("Aimbot")) {
                    ImGui::Spacing();
                    ImGui::Checkbox("Enable Triggerbot", &triggerbotEnabled);
                    if (triggerbotEnabled) {
                        ImGui::Indent(16);
                        ImGui::Checkbox("Triggerbot Team Check", &triggerbotTeamCheck);
                        ImGui::SliderInt("Triggerbot Delay (ms)", &triggerbotDelay, 1, 200);
                        ImGui::Text("Key"); ImGui::SameLine();
                        ImGui::PushID("TrigKey");
                        if (ImGui::Button(triggerbotKey != ImGuiKey_None ? ImGui::GetKeyName(triggerbotKey) : "None", ImVec2(100,0)))
                            ImGui::OpenPopup("SetTrigKey");
                        if (ImGui::BeginPopup("SetTrigKey")) {
                            ImGui::Text("Press a key...");
                            for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; k++)
                                if (ImGui::IsKeyPressed((ImGuiKey)k, false)) {
                                    triggerbotKey = (ImGuiKey)k;
                                    triggerbotVkCode = 0;
                                    for (int vk = 0x08; vk <= 0xFE; vk++)
                                        if (GetAsyncKeyState(vk) & 0x8000) { triggerbotVkCode = vk; break; }
                                    ImGui::CloseCurrentPopup();
                                }
                            ImGui::EndPopup();
                        }
                        ImGui::PopID();
                        ImGui::SameLine();
                        const char* modes[] = {"Always", "Hold", "Toggle"};
                        ImGui::Combo("##mode", &triggerbotMode, modes, 3);
                        ImGui::Unindent(16);
                    }
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    ImGui::Checkbox("Enable Aimbot", &aimbotEnabled);
                    if (aimbotEnabled) {
                        ImGui::Indent(16);
                        ImGui::Text("Key"); ImGui::SameLine();
                        ImGui::PushID("AimKey");
                        if (ImGui::Button(aimbotKey != ImGuiKey_None ? ImGui::GetKeyName(aimbotKey) : "None", ImVec2(100,0)))
                            ImGui::OpenPopup("SetAimKey");
                        if (ImGui::BeginPopup("SetAimKey")) {
                            ImGui::Text("Press a key...");
                            for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; k++)
                                if (ImGui::IsKeyPressed((ImGuiKey)k, false)) {
                                    aimbotKey = (ImGuiKey)k;
                                    aimbotVkCode = 0;
                                    for (int vk = 0x08; vk <= 0xFE; vk++)
                                        if (GetAsyncKeyState(vk) & 0x8000) { aimbotVkCode = vk; break; }
                                    ImGui::CloseCurrentPopup();
                                }
                            ImGui::EndPopup();
                        }
                        ImGui::PopID();
                        ImGui::SameLine();
                        const char* aimModes[] = {"Always", "Hold", "Toggle"};
                        ImGui::Combo("##aimMode", &aimbotMode, aimModes, 3);
                        ImGui::SliderFloat("Aimbot FOV", &aimbotFov, 0.f, 180.f, "%.1f");
                        ImGui::SliderFloat("Smooth", &aimbotSmooth, 1.f, 20.f, "%.1f");
                        ImGui::SliderFloat("H-Offset", &aimbotAimX, -5.f, 5.f, "%.1f");
                        ImGui::SliderFloat("V-Offset", &aimbotAimY, -5.f, 5.f, "%.1f");
                        ImGui::Checkbox("Aim Head", &aimbotHead);
                        ImGui::Checkbox("Aim Chest", &aimbotChest);
                        ImGui::Checkbox("Draw FOV", &aimbotDrawFov);
                        ImGui::Checkbox("Team Check", &aimbotTeamCheck);
                        ImGui::Unindent(16);
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Visuals")) {
                    ImGui::Spacing();
                    ImGui::Checkbox("Enable ESP", &espEnabled);
                    if (espEnabled) {
                        ImGui::Indent(16);
                        ImGui::Checkbox("Box", &boxEnabled);
                        ImGui::SameLine(120);
                        if (ImGui::ColorButton("Box##c", ImVec4(boxColor.r,boxColor.g,boxColor.b,1)))
                            ImGui::OpenPopup("BoxColor");
                        if (ImGui::BeginPopup("BoxColor")) {
                            ImGui::ColorPicker3("##box", (float*)&boxColor); ImGui::EndPopup();
                        }
                        ImGui::Checkbox("Health Bar", &healthEnabled);
                        ImGui::SameLine(120);
                        if (ImGui::ColorButton("Health##c", ImVec4(healthColor.r,healthColor.g,healthColor.b,1)))
                            ImGui::OpenPopup("HealthColor");
                        if (ImGui::BeginPopup("HealthColor")) {
                            ImGui::ColorPicker3("##hlth", (float*)&healthColor); ImGui::EndPopup();
                        }
                        ImGui::Checkbox("Name", &NameEsp);
                        ImGui::SameLine(120);
                        if (ImGui::ColorButton("Name##c", ImVec4(nameColor.r,nameColor.g,nameColor.b,1)))
                            ImGui::OpenPopup("NameColor");
                        if (ImGui::BeginPopup("NameColor")) {
                            ImGui::ColorPicker3("##name", (float*)&nameColor); ImGui::EndPopup();
                        }
                        ImGui::Checkbox("Weapon", &WeaponEsp);
                        ImGui::SameLine(120);
                        if (ImGui::ColorButton("Weapon##c", ImVec4(weaponColor.r,weaponColor.g,weaponColor.b,1)))
                            ImGui::OpenPopup("WeaponColor");
                        if (ImGui::BeginPopup("WeaponColor")) {
                            ImGui::ColorPicker3("##wp", (float*)&weaponColor); ImGui::EndPopup();
                        }
                        ImGui::Checkbox("Skeleton", &skeletonEnabled);
                        ImGui::SameLine(120);
                        if (ImGui::ColorButton("Skeleton##c", ImVec4(skeletonColor.r,skeletonColor.g,skeletonColor.b,1)))
                            ImGui::OpenPopup("SkeletonColor");
                        if (ImGui::BeginPopup("SkeletonColor")) {
                            ImGui::ColorPicker3("##skel", (float*)&skeletonColor); ImGui::EndPopup();
                        }
                        ImGui::SliderFloat("Skeleton Thickness", &skeletonThickness, 1.f, 5.f, "%.1f");
                        ImGui::Checkbox("Team Check", &teammateEsp);
                        ImGui::Unindent(16);
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Misc")) {
                    ImGui::Spacing();
                    ImGui::Checkbox("Watermark", &watermarkEnabled);
                    ImGui::Checkbox("No Flash", &noFlashEnabled);
                    ImGui::Checkbox("Radar", &radarEnabled);
                    if (radarEnabled) { ImGui::Indent(16); ImGui::SliderFloat("Zoom", &radarZoom, 500.f, 5000.f, "%.0f"); ImGui::Unindent(16); }
                    ImGui::Spacing();
                    {
                        const char* preview = configNames.empty() ? "default" : configNames[currentConfig < 0 ? 0 : currentConfig].c_str();
                        if (ImGui::BeginCombo("Config", preview)) {
                            for (int i = 0; i < (int)configNames.size(); i++) {
                                bool sel = (i == currentConfig);
                                if (ImGui::Selectable(configNames[i].c_str(), &sel)) { currentConfig = i; LoadConfig(); }
                                if (sel) ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Refresh")) RefreshConfigs();
                        if (ImGui::Button("Save", ImVec2(100, 0))) SaveConfig();
                        ImGui::SameLine();
                        if (ImGui::Button("Load", ImVec2(100, 0))) LoadConfig();
                        ImGui::InputText("##newcfg", newConfigName, sizeof(newConfigName));
                        ImGui::SameLine();
                        if (ImGui::Button("New")) {
                            if (newConfigName[0]) {
                                configNames.push_back(newConfigName);
                                currentConfig = (int)configNames.size() - 1;
                                SaveConfig();
                                newConfigName[0] = '\0';
                            }
                        }
                    }
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();

            {
                ImVec2 wp = ImGui::GetWindowPos(), ws = ImGui::GetWindowSize();
                std::string lbl = std::string("Build: ") + __DATE__;
                ImGui::GetWindowDrawList()->AddText(ImVec2(wp.x+ws.x-ImGui::CalcTextSize(lbl.c_str()).x-14, wp.y+ws.y-20),
                    IM_COL32(100,100,100,255), lbl.c_str());
            }
            ImGui::End();
        }

        if (watermarkEnabled) {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, watermarkBg);
            ImGui::SetNextWindowBgAlpha(0.5f);
            ImGui::SetNextWindowPos(ImVec2((float)sw-10, 10), ImGuiCond_Always, ImVec2(1,0));
            ImGui::Begin("##wm", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoCollapse);
            auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            struct tm ti; localtime_s(&ti, &t); char tb[32]; strftime(tb,sizeof(tb),"%H:%M:%S",&ti);
            std::string wt = std::string("Black0ut / ") + tb + " / FPS: " + std::to_string(fps) + " / Ping: " + std::to_string(ping) + "ms";
            ImGui::TextColored(watermarkText, "%s", wt.c_str());
            ImGui::End();
            ImGui::PopStyleColor();
        }

        if (espEnabled && localPawn && localHP > 0 && localHP <= 100 && list) {
            for (int i = 1; i <= 64; i++) {
                uintptr_t e = g_mem.Read<uintptr_t>(list + 16 + 8*(i>>9)); if (!e) continue;
                uintptr_t ctrl = g_mem.Read<uintptr_t>(e + 0x70*(i&0x1FF)); if (!ctrl) continue;
                uint32_t pawnHandle = g_mem.Read<uint32_t>(ctrl + controller_offsets::m_hPlayerPawn); if (!pawnHandle) continue;
                uintptr_t pe = g_mem.Read<uintptr_t>(list + 16 + 8*((pawnHandle&0x7FFF)>>9)); if (!pe) continue;
                uintptr_t pawn = g_mem.Read<uintptr_t>(pe + 0x70*(pawnHandle&0x1FF)); if (!pawn || pawn == localPawn) continue;

                int hp = g_mem.Read<int>(pawn + pawn_offsets::m_iHealth); if (hp <= 0 || hp > 100) continue;
                int team = g_mem.Read<int>(pawn + pawn_offsets::m_iTeamNum);
                if (teammateEsp && team == localTeam) continue;
                if (localTeam != 2 && localTeam != 3) continue;

                uintptr_t sn = g_mem.Read<uintptr_t>(pawn + pawn_offsets::m_pGameSceneNode); if (!sn) continue;
                Vector3 origin = g_mem.Read<Vector3>(pawn + pawn_offsets::m_vOldOrigin);
                Vector3 headPos = origin; headPos.z += 72.f;

                Vector2 scrO, scrH;
                if (!WorldToScreen(origin, scrO, vm, sw, sh)) continue;
                if (!WorldToScreen(headPos, scrH, vm, sw, sh)) continue;

                float height = scrO.y - scrH.y; if (height < 10.f) continue;
                float width = height * 0.45f;
                float bx = scrO.x - width/2, by = scrH.y;

                if (boxEnabled) Render::DrawRect((int)bx, (int)by, (int)width, (int)height, boxColor, 1);

                if (healthEnabled) {
                    float barH = height * (hp / 100.f);
                    float barX = bx - 5.f;
                    Render::Line(barX, by+height, barX, by, {0.4f,0.4f,0.4f}, 2.f);
                    Render::Line(barX, by+height, barX, by+height-barH, healthColor, 2.f);
                }

                if (NameEsp) {
                    char name[128]={};
                    uintptr_t namePtr = g_mem.Read<uintptr_t>(ctrl + controller_offsets::m_sSanitizedPlayerName);
                    if (namePtr) g_mem.ReadBuffer(namePtr, name, sizeof(name)-1);
                    if (name[0]) Render::DrawText(bx, by-14, name, nameColor);
                }

                if (WeaponEsp) {
                    const char* wn = GetWeaponName(pawn, list);
                    Render::DrawText(bx, scrO.y+4, wn, weaponColor);
                }

                if (skeletonEnabled) {
                    uintptr_t boneArr = g_mem.Read<uintptr_t>(sn + scene_offsets::m_modelState + 0x80);
                    if (boneArr) {
                        Vector3 headBone = g_mem.Read<Vector3>(boneArr + 6 * 32); headBone.z += 5.f;
                        if (headBone.IsValid()) {
                            Vector2 scHead;
                            if (WorldToScreen(headBone, scHead, vm, sw, sh))
                                Render::Circle(scHead.x, scHead.y, width * 0.16f, skeletonColor);
                        }
                        for (const auto& conn : boneConnections) {
                            Vector3 b1 = g_mem.Read<Vector3>(boneArr + conn.bone1 * 32);
                            Vector3 b2 = g_mem.Read<Vector3>(boneArr + conn.bone2 * 32);
                            if (b1.IsValid() && b2.IsValid()) {
                                Vector2 s1, s2;
                                if (WorldToScreen(b1, s1, vm, sw, sh) && WorldToScreen(b2, s2, vm, sw, sh))
                                    Render::Line(s1.x, s1.y, s2.x, s2.y, skeletonColor, skeletonThickness);
                            }
                        }
                    }
                }
            }
        }

        if (radarEnabled && localPawn && list) {
            Vector3 localOrigin = g_mem.Read<Vector3>(localPawn + pawn_offsets::m_vOldOrigin);
            Vector3 viewAngles = g_mem.Read<Vector3>(g_client + cs2_dumper::offsets::client_dll::dwViewAngles);
            float yaw = viewAngles.y * (3.14159265f / 180.f);
            float radarSize = 160.f, gap = 15.f;
            float rx = gap, ry = sh - radarSize - gap;
            float cx = rx + radarSize/2, cy = ry + radarSize/2;
            float scale = radarSize / radarZoom;
            auto* dl = ImGui::GetBackgroundDrawList();
            dl->AddRectFilled(ImVec2(rx,ry), ImVec2(rx+radarSize,ry+radarSize), IM_COL32(0,0,0,160));
            dl->AddRect(ImVec2(rx,ry), ImVec2(rx+radarSize,ry+radarSize), IM_COL32(255,255,255,40));
            dl->AddTriangleFilled(ImVec2(cx,cy-6), ImVec2(cx-4,cy+4), ImVec2(cx+4,cy+4), IM_COL32(0,200,255,220));
            uintptr_t drawn[64] = {}; int drawnCount = 0;
            auto drawPawn = [&](uintptr_t pawn, int team, Vector3 pos) {
                for (int d = 0; d < drawnCount; d++) if (drawn[d] == pawn) return;
                if (drawnCount >= 64) return;
                float dx = pos.x - localOrigin.x, dy = pos.y - localOrigin.y;
                float dist = sqrtf(dx*dx + dy*dy); if (dist > 5000.f) return;
                float forward = dx*cosf(yaw) + dy*sinf(yaw);
                float right   = dx*sinf(yaw) - dy*cosf(yaw);
                float sx = cx + right*scale, sy = cy - forward*scale;
                if (sx < rx-5 || sx > rx+radarSize+5 || sy < ry-5 || sy > ry+radarSize+5) return;
                dl->AddCircleFilled(ImVec2(sx,sy), 3.f, (team == localTeam) ? IM_COL32(0,255,0,200) : IM_COL32(255,0,0,200));
                drawn[drawnCount++] = pawn;
            };
            for (int i = 1; i <= 64; i++) {
                uintptr_t e = g_mem.Read<uintptr_t>(list + 16 + 8*(i>>9)); if (!e) continue;
                uintptr_t ctrl = g_mem.Read<uintptr_t>(e + 0x70*(i&0x1FF)); if (!ctrl) continue;
                uint32_t ph = g_mem.Read<uint32_t>(ctrl + controller_offsets::m_hPlayerPawn); if (!ph) continue;
                uintptr_t pe = g_mem.Read<uintptr_t>(list + 16 + 8*((ph&0x7FFF)>>9)); if (!pe) continue;
                uintptr_t pawn = g_mem.Read<uintptr_t>(pe + 0x70*(ph&0x1FF)); if (!pawn || pawn == localPawn) continue;
                drawPawn(pawn, g_mem.Read<int>(pawn + pawn_offsets::m_iTeamNum), g_mem.Read<Vector3>(pawn + pawn_offsets::m_vOldOrigin));
            }
            for (int base_off = 0; base_off <= 16; base_off += 8) {
                uintptr_t array = g_mem.Read<uintptr_t>(list + base_off); if (!array) continue;
                for (int ci = 0; ci < 4; ci++) {
                    uintptr_t chunk = g_mem.Read<uintptr_t>(array + (ci << 3)); if (!chunk) continue;
                    for (int si = 1; si < 512; si++) {
                        uintptr_t pawn = g_mem.Read<uintptr_t>(chunk + 0x70*si);
                        if (!pawn || pawn == localPawn || pawn < 0x10000 || (pawn >> 48)) continue;
                        uint32_t ctrlHandle = g_mem.Read<uint32_t>(pawn + pawn_offsets::m_hController); if (!ctrlHandle) continue;
                        drawPawn(pawn, g_mem.Read<int>(pawn + pawn_offsets::m_iTeamNum), g_mem.Read<Vector3>(pawn + pawn_offsets::m_vOldOrigin));
                    }
                }
            }
        }

        if (aimbotDrawFov) {
            float radius = tanf(aimbotFov * 3.14159265f / 360.f) * (sw/2.f);
            ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(sw/2.f, sh/2.f), radius,
                IM_COL32(0,0,0,80), 48, 2.f);
        }

        ImGui::Render();
        float clear[4] = {0,0,0,0};
        ctx->OMSetRenderTargets(1, &target, nullptr);
        ctx->ClearRenderTargetView(target, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        swap->Present(0, 0);
        Sleep(1);
    }

    ImGui_ImplDX11_Shutdown(); ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext();
    if (target) target->Release();
    if (swap) swap->Release();
    if (ctx) ctx->Release();
    if (device) device->Release();
    DestroyWindow(overlay);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}
