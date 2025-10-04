# RPG SFML Game

A minimal starting point for a C++20 RPG project using **SFML 3** and **CMake**.

---

## 🧱 Build

### Prerequisites
- CMake ≥ 3.28
- A C++20 compiler (GCC 13+, Clang 16+, or MSVC 2022)
- [Ninja](https://ninja-build.org/) (recommended; used by the included presets)

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
````

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

## 📜 License

Licensed under the MIT License — see [LICENSE](LICENSE) for details.
