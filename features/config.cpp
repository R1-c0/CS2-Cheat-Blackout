#include <Windows.h>
#include <string>
#include <fstream>
#include <vector>
#include "imgui/imgui.h"
#include "globals.h"
#include "features.h"

std::string ConfigDir() {
    char buf[MAX_PATH]; GetModuleFileNameA(nullptr, buf, sizeof(buf));
    std::string exe(buf); auto pos = exe.find_last_of("\\/");
    return exe.substr(0, pos) + "\\configs\\";
}
std::string ConfigPath(const std::string& name) {
    return ConfigDir() + name + ".cfg";
}
std::string CurrentConfigPath() {
    if (configNames.empty() || currentConfig < 0) return ConfigPath("default");
    return ConfigPath(configNames[currentConfig]);
}
void RefreshConfigs() {
    configNames.clear(); currentConfig = -1;
    std::string dir = ConfigDir();
    WIN32_FIND_DATAA fd; HANDLE h = FindFirstFileA((dir + "*.cfg").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do { std::string n(fd.cFileName); configNames.push_back(n.substr(0, n.size()-4)); }
        while (FindNextFileA(h, &fd));
        FindClose(h);
    }
    if (configNames.empty()) { configNames.push_back("default"); currentConfig = 0; }
    else currentConfig = 0;
}
void SaveConfig() {
    if (configNames.empty() || currentConfig < 0) return;
    std::string dir = ConfigDir();
    CreateDirectoryA(dir.c_str(), nullptr);
    std::ofstream f(CurrentConfigPath());
    if (!f) return;
    int tmpKey = (int)triggerbotKey;
    int aimKey = (int)aimbotKey;
    f << espEnabled << ' ' << boxEnabled << ' ' << healthEnabled << ' ' << NameEsp << ' '
      << WeaponEsp << ' ' << skeletonEnabled << ' ' << teammateEsp << ' '
      << triggerbotEnabled << ' ' << triggerbotDelay << ' ' << triggerbotTeamCheck << ' ' << tmpKey << ' ' << triggerbotVkCode << ' ' << triggerbotMode << ' '
      << noFlashEnabled << ' ' << watermarkEnabled << ' ' << radarEnabled << ' ' << radarZoom << ' ' << jumpThrowEnabled << ' ' << fakeAnglesEnabled << ' '
      << aimbotFov << ' ' << aimbotSmooth << ' ' << aimbotHead << ' ' << aimbotChest << ' ' << aimbotEnabled << ' ' << aimKey << ' ' << aimbotVkCode << ' ' << aimbotMode << ' ' << aimbotDrawFov << ' ' << aimbotTeamCheck << ' ' << aimbotAimX << ' ' << aimbotAimY;
}
void LoadConfig() {
    std::ifstream f(CurrentConfigPath());
    if (!f) return;
    int tmpKey = 0; int tmpVk = 0; int aimKey = 0;
    f >> espEnabled >> boxEnabled >> healthEnabled >> NameEsp
      >> WeaponEsp >> skeletonEnabled >> teammateEsp
      >> triggerbotEnabled >> triggerbotDelay >> triggerbotTeamCheck >> tmpKey >> tmpVk >> triggerbotMode
      >> noFlashEnabled >> watermarkEnabled >> radarEnabled >> radarZoom >> jumpThrowEnabled >> fakeAnglesEnabled
      >> aimbotFov >> aimbotSmooth >> aimbotHead >> aimbotChest >> aimbotEnabled >> aimKey >> aimbotVkCode >> aimbotMode >> aimbotDrawFov >> aimbotTeamCheck >> aimbotAimX >> aimbotAimY;
    triggerbotKey = (ImGuiKey)tmpKey;
    triggerbotVkCode = tmpVk;
    aimbotKey = (ImGuiKey)aimKey;
}
