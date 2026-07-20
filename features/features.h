#pragma once
void HandleNoFlash();
void HandleJumpThrow();
void HandleFakeAngles();
void HandleTriggerBot();
void HandleAimbot();
std::string ConfigDir();
std::string ConfigPath(const std::string& name);
std::string CurrentConfigPath();
void RefreshConfigs();
void SaveConfig();
void LoadConfig();
