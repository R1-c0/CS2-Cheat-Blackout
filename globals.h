#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "imgui/imgui.h"
#include "memory.h"
#include "sdk/structs.h"
#include "render.h"

extern Memory g_mem;
extern std::uintptr_t g_client;

extern bool guiActive;
extern bool espEnabled;
extern bool boxEnabled;
extern bool healthEnabled;
extern bool NameEsp;
extern bool WeaponEsp;
extern bool skeletonEnabled;
extern bool teammateEsp;
extern bool triggerbotEnabled;
extern int  triggerbotDelay;
extern bool triggerbotTeamCheck;
extern int  triggerbotVkCode;
extern ImGuiKey triggerbotKey;
extern int  triggerbotMode;
extern bool noFlashEnabled;
extern bool watermarkEnabled;
extern bool radarEnabled;
extern float radarZoom;
extern bool jumpThrowEnabled;
extern ImGuiKey jumpThrowKey;
extern bool fakeAnglesEnabled;
extern bool aimbotEnabled;
extern float aimbotFov;
extern float aimbotSmooth;
extern bool aimbotHead;
extern bool aimbotChest;
extern int  aimbotVkCode;
extern ImGuiKey aimbotKey;
extern int  aimbotMode;
extern bool aimbotDrawFov;
extern bool aimbotTeamCheck;
extern float aimbotAimX;
extern float aimbotAimY;
extern RGBs boxColor, nameColor, weaponColor, skeletonColor, healthColor, healTextColor;
extern float skeletonThickness;
extern int sw, sh;
extern std::vector<std::string> configNames;
extern int currentConfig;
extern char newConfigName[64];
