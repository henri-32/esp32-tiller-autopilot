# Autopilot (PlatformIO, STM32F411RE)

This project is built with PlatformIO. The repository contains a clangd setup so code intelligence works on Windows and Linux.

**Build**
1. Open the project folder in VSCode.
2. Run `PlatformIO: Build` or use CLI:
```bash
pio run
```

**Build and Test Matrix**
| Command | Purpose | Expected result |
|---|---|---|
| `pio run -e genericSTM32F411RE` | Build firmware for STM32 target | Build succeeds |
| `pio run -e native` | Build native host executable (`src/native_main.cpp`) | Build succeeds |
| `pio test -e native_csc` | Run CSC unit tests (Unity) | Tests build and run |
| `pio run -e native_csc` | Not a valid workflow (test-only env) | Fails with missing `main` (expected) |

`env:native_csc` is intentionally test-only and should always be used with `pio test`.

**clangd (code intelligence)**
clangd uses `compile_commands.json`. Generate it after changing `platformio.ini`, board, or `build_flags`.

Windows (PowerShell):
```powershell
C:\Users\henri\.platformio\penv\Scripts\pio.exe run -t compiledb
```

Linux:
```bash
pio run -t compiledb
```

**Notes for multi-OS development**
- `compile_commands.json` is generated per machine and should not be committed if it causes churn.
- The project includes a `.clangd` that forces GCC version macros for the STM32 toolchain. This is needed so clangd does not trigger the `GCC version 6.3 or higher is required` error from `Arduino.h`.
- In VSCode, `clangd.arguments` are set in `.vscode/settings.json` and point to the local PlatformIO GCC. If your PlatformIO path differs, update those paths.

**VSCode task**
There is a task to regenerate the compilation database:
1. `Ctrl+Shift+P` → `Tasks: Run Task`
2. `PlatformIO: Generate compile_commands.json`
