# Black0ut CS2

External overlay for Counter-Strike 2, built with ImGui + DirectX 11.

## Features

- **ESP** – Box, Health Bar, Name, Weapon, Skeleton (bone connections), Team Check
- **Triggerbot** – Always/Hold/Toggle, adjustable delay, Team Check
- **Aimbot** – Always/Hold/Toggle, Smooth, FOV, Head/Chest selection, H/V-Offset, Draw FOV, Team Check
- **Radar** – 2D radar with rotation, zoom slider, two-pass scan (controller→pawn + direct scan)
- **Misc** – No Flash, Watermark, multi-config system (Save/Load/New)

## Build

1. **Visual Studio 2022** with *Desktop development with C++* workload installed
2. Run `build.bat`
3. Output: `build\Black0ut.exe`

## Usage

1. Launch `cs2.exe`
2. Run `build\Black0ut.exe` as Administrator
3. `INSERT` – toggle GUI
4. Configs are stored in the `configs/` folder

## Project Structure

```
├── build.bat                  # Build script
├── main.cpp                   # Overlay, ESP, Radar, UI
├── globals.h / globals.cpp    # Shared variables
├── memory.h / memory.cpp      # External memory read/write
├── render.h                   # Drawing helpers (Text, Line, Circle, Rect)
├── features/
│   ├── triggerbot.cpp         # Triggerbot thread
│   ├── aimbot.cpp             # Aimbot thread
│   ├── misc.cpp               # NoFlash, JumpThrow, FakeAngles
│   └── config.cpp             # Config Save/Load
├── sdk/
│   ├── offsets.h              # Pawn, Controller, Weapon, Scene offsets
│   └── structs.h              # Vector3, view_matrix_t, etc.
├── cs/
│   ├── bone.hpp               # Bone indices + connections
│   └── weapon_index.h         # Weapon ID → name mapping
└── imgui/                     # ImGui (v1.91.6)
```

## Notes

- W2S uses column-major (matrix * vector) – matches CS2/D3D11 view matrix storage.
- CUtlString (`m_sSanitizedPlayerName`) is a `char*` pointer – must be dereferenced before reading.
- Triggerbot reads `m_iIDEntIndex` directly from the pawn.
- Configurations are stored as `.cfg` files in `configs/`.
