@echo off
cd /d "%~dp0"
set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist %VSWHERE% (echo VS not found & pause & exit /b 1)
for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -property installationPath`) do set VS_PATH=%%i
call "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x64
if not exist build mkdir build
del /q build\*.obj build\*.exe 2>nul
echo Compiling Black0ut.exe ...
cl /nologo /O2 /MD /EHsc /std:c++17 /Fo"build\\" /Fe"build\Black0ut.exe" ^
    /I"imgui" /I"imgui/backends" /I"sdk" /I"cs" /I"features" /I"." ^
    main.cpp memory.cpp globals.cpp ^
    features/triggerbot.cpp features/aimbot.cpp features/misc.cpp features/config.cpp ^
    imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp imgui/imgui_tables.cpp ^
    imgui/backends/imgui_impl_dx11.cpp imgui/backends/imgui_impl_win32.cpp ^
    /link /SUBSYSTEM:CONSOLE d3d11.lib dxgi.lib dwmapi.lib
if %errorlevel% equ 0 (echo Build succeeded: build\Black0ut.exe) else (echo Build failed & pause)
