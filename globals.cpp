#include "globals.h"

Memory g_mem;
std::uintptr_t g_client = 0;

bool guiActive = false;
bool espEnabled = false;
bool boxEnabled = false;
bool healthEnabled = false;
bool NameEsp = false;
bool WeaponEsp = false;
bool skeletonEnabled = false;
bool teammateEsp = true;
bool triggerbotEnabled = false;
int  triggerbotDelay = 30;
bool triggerbotTeamCheck = false;
int  triggerbotVkCode = 0;
ImGuiKey triggerbotKey = ImGuiKey_None;
int  triggerbotMode = 0;
bool noFlashEnabled = false;
bool watermarkEnabled = false;
bool radarEnabled = false;
float radarZoom = 2500.f;
bool jumpThrowEnabled = false;
ImGuiKey jumpThrowKey = ImGuiKey_None;
bool fakeAnglesEnabled = false;
bool aimbotEnabled = false;
float aimbotFov = 15.f;
float aimbotSmooth = 5.f;
bool aimbotHead = true;
bool aimbotChest = false;
int  aimbotVkCode = 0;
ImGuiKey aimbotKey = ImGuiKey_None;
int  aimbotMode = 0;
bool aimbotDrawFov = false;
bool aimbotTeamCheck = true;
float aimbotAimX = 0.f;
float aimbotAimY = 0.f;

RGBs boxColor         = {0.86f, 0.86f, 0.86f};
RGBs nameColor        = {0.86f, 0.86f, 0.86f};
RGBs weaponColor      = {0.86f, 0.86f, 0.86f};
RGBs skeletonColor    = {0.86f, 0.86f, 0.86f};
RGBs healthColor      = {0.86f, 0.86f, 0.86f};
RGBs healTextColor    = {0.86f, 0.86f, 0.86f};
float skeletonThickness = 2.0f;

int sw = 0, sh = 0;
std::vector<std::string> configNames;
int currentConfig = -1;
char newConfigName[64] = {};
