# RPG SFML Game

A minimal starting point for a C++20 RPG project using **SFML 3** and **CMake**.

---

## 🧱 Build

### Prerequisites

* CMake ≥ 3.28
* A C++20 compiler (GCC 13+, Clang 16+, or MSVC 2022)
* [Ninja](https://ninja-build.org/) (recommended; used by the included presets)

On **Linux**, you may need SFML’s system dependencies (graphics/audio/windowing):

```bash
sudo apt update
sudo apt install \
  libx11-dev \
  libxrandr-dev \
  libxcursor-dev \
  libxi-dev \
  libudev-dev \
  libgl1-mesa-dev \
  libegl1-mesa-dev \
  libfreetype-dev \
  libsndfile1-dev \
  libopenal-dev \
  libflac-dev \
  libvorbis-dev
```

> macOS typically needs no extra packages when building via CMake + Xcode/Clang.

---

### Configure & Build (with presets)

**RelWithDebInfo (default “dev”)**

```bash
cmake --preset dev
cmake --build --preset build -j
```

**Debug**

```bash
cmake --preset debug
cmake --build --preset build-debug -j
```

---

## ▶️ Run

**Linux/macOS**

```bash
./build/bin/rpg
```

**Windows (with Ninja, as per presets)**

```powershell
.\build\bin\rpg.exe
```

> If you configure with the **Visual Studio** generator instead of Ninja, the output will be under a config subfolder, e.g.:
> `.\build\bin\RelWithDebInfo\rpg.exe`

---

## 🧰 Development Utilities

### 🔧 Code Style & Formatting

This project enforces a consistent code style across editors and CI:

| Tool               | Purpose                                                         | Location         |
| ------------------ | --------------------------------------------------------------- | ---------------- |
| **clang-format**   | Formats all C++ sources (`.cpp`, `.hpp`, etc.)                  | `.clang-format`  |
| **EditorConfig**   | Consistent indentation, newlines, and whitespace across editors | `.editorconfig`  |
| **Git Attributes** | Normalizes line endings and treats binary assets correctly      | `.gitattributes` |

Format all sources:

```bash
cmake --build build --target rpg-tools-format
```

Fail the build if formatting is off (no changes applied):

```bash
cmake --build build --target rpg-tools-format-check
```

Override the formatter binary if needed (e.g., on Ubuntu 24.04):

```bash
cmake -S . -B build -DCLANG_FORMAT_EXECUTABLE=clang-format-18
```

Make sure your editor has **EditorConfig** and **clang-format** integration enabled.

---

### 🔍 Static Analysis (clang-tidy)

A convenience target runs clang-tidy over **your project sources** (`include/`, `src/`).
Third-party/vendor directories like `_deps/`, `extlibs/`, and `third-party/` are **excluded**.

Run it:

```bash
cmake --build build --target rpg-tools-tidy -j
```

Requirements:

* CMake preset must generate `compile_commands.json`. (Already enabled in presets; if not, set `CMAKE_EXPORT_COMPILE_COMMANDS=ON`.)
* Clang-Tidy ≥ 14 (tested with 20).

Options:

```bash
# Use a specific clang-tidy binary
cmake -S . -B build -DCLANG_TIDY_EXECUTABLE=clang-tidy-20
cmake --build build --target rpg-tools-tidy
```

Tips:

* If you see `'stddef.h' file not found`, make sure a matching **clang** is installed. The tooling auto-detects the clang resource dir and passes it to clang-tidy.

---

### 🪝 Git Hooks

Local Git hooks keep code style consistent:

* **pre-commit**: Formats staged C/C++ files with `clang-format` and re-stages them.
* **pre-push** (optional): Runs the no-changes style check before pushing.

Install once:

```bash
bash tools/install-git-hooks.sh
```

Override the formatter per command (examples):

```bash
# Use a specific clang-format during commit
CLANG_FORMAT=/usr/bin/clang-format-18 git commit -m "feat: …"

# Configure build/check with a specific formatter
cmake -S . -B build -DCLANG_FORMAT_EXECUTABLE=clang-format-18
cmake --build build --target rpg-tools-format-check
```

---

## 📜 License

Licensed under the MIT License — see [LICENSE](LICENSE) for details.
