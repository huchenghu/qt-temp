# AGENTS.md

## Build & Test

- Full configure+build+test in one step: `cmake --workflow --preset debug`
- Step-by-step: `cmake --preset debug && cmake --build --preset debug && ctest --preset debug`
- Presets: `debug`, `release`, `relwithdebinfo`. Uses Ninja + clang by default.
- Headless test: set `QT_QPA_PLATFORM=offscreen` (needed in CI, no display).
- Run single test: `ctest --preset debug -R test_name` (names: `test_common_utils`, `test_config_manager`, `test_mpsc_queue`).
- To rebuild from clean: `cmake --build --preset debug` already does `--clean-first`.

## Code Style & Tooling

- **Formatting**: `clang-format` based on LLVM style. Config at `.clang-format`. Allman braces, 120 col limit, 2-space indent, no tabs. CI blocks unformatted code.
- **Linting**: `clang-tidy` used in CI (first 5 `.cpp` files in `libs/ apps/`). `.clangd` enables broad tidy checks + strict unused/missing includes diagnostics.
- **C++17**, `#pragma once`, `QtUtils` namespace for library code.

## Project Structure

```
├── libs/qtutils/          Static library (qtutils), consumed by all apps
│   ├── include/qtutils/   Public headers
│   ├── src/               Implementation
│   └── tests/             QtTest-based unit tests (CTest integration)
├── apps/app/              GUI application (Qt Widgets)
├── apps/log-bench/        CLI benchmark tool for LogManager (Qt Core only)
```

- `libs/qtutils` is a `STATIC` library. Links Qt{Core,Gui,Network,Widgets,Concurrent}.
- `apps/app` and `apps/log-bench` link `qtutils`. Add new apps under `apps/` and register in `apps/CMakeLists.txt`.
- `CMAKE_AUTOMOC`, `CMAKE_AUTOUIC`, `CMAKE_AUTORCC` are ON.

## Dependencies

- Qt6 (can fall back to Qt5). Required components vary per target (see CMakeLists.txt files).
- Compiler: clang (default), gcc, msvc presets available.
- Generator: Ninja (default). Unix Makefiles and VS 17 2022 presets also defined.
- Optional `vcpkg` via toolchain preset (`VCPKG_CUSTOM` env var) or Qt6 toolchain preset (`QT6_CUSTOM` / `QT5_CUSTOM` env vars).
- `.clangd` expects compilation database at `./build/debug`.
