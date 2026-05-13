# Agent Guidance for Magic Set Editor 2

This repository is a native C++ application built with CMake. The source tree is organized into a shared core plus two optional GUI backends.

## Key facts

- Language: C++17
- Build system: CMake
- Primary GUI backend: wxWidgets
- Optional GUI backend: Qt6 (`-DUSE_QT6=ON`)
- Required libraries: Boost (regex), Hunspell, wxWidgets or Qt6
- Entry points: `src/main.cpp`, `src/cli/cli_main.cpp`
- Core code: `src/data`, `src/gfx`, `src/render`, `src/script`, `src/util`, `src/gui_core`
- GUI backends: `src/gui` + `src/gui_wx` for wxWidgets, `src/gui_qt` for Qt6
- Resource and platform-specific files: `resource/`, `resource/win32_res.rc`, `MacOSXBundleInfo.plist.in`

## Build and test commands

Use the repository root as the working directory.

Linux / macOS:

```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

Enable Qt6 backend:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DUSE_QT6=ON
cmake --build build
```

Windows Visual Studio / vcpkg: use the `README.md` instructions for installing dependencies and open the folder in Visual Studio. The project uses `CMakeLists.txt` directly.

Tests are configured in `test/tests.cmake` and included via `CMakeLists.txt`.

## Repository conventions

- Prefer editing common core code in `src/data`, `src/render`, `src/script`, and `src/util`.
- GUI-specific changes should go into `src/gui`, `src/gui_wx`, or `src/gui_qt` depending on the backend.
- `src/config.hpp` is generated from `src/config.hpp.in` at configure time.
- The build toggles the Qt backend via the `USE_QT6` option.
- Hunspell is discovered with pkg-config, falling back to manual find paths.

## Useful references

- See `README.md` for platform-specific dependency and build guidance.
- See `CMakeLists.txt` for dependency logic, compiler flags, and target setup.

## Notes for agents

- Do not assume a single GUI framework: preserve both wxWidgets and Qt6 paths unless a change explicitly targets one backend.
- Focus on existing CMake patterns rather than inventing a new build system.
- Keep changes minimal and compatible with both Linux and Windows build flows when possible.
