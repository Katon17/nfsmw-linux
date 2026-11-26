# Need for Speed: Most Wanted (Black Edition) — Native Linux Port

🇮🇩 **Bahasa Indonesia**: lihat [README.id.md](README.id.md)

[![Release: v0.1.13](https://img.shields.io/badge/Release-v0.1.13-blue.svg)](https://github.com/Katon17/nfsmw-linux/releases)
[![Platform: Linux x86_64](https://img.shields.io/badge/Platform-Linux%20x86__64-orange.svg)](https://kernel.org)
[![Target: Linux Mint / Ubuntu / Debian](https://img.shields.io/badge/Target-Linux%20Mint%20%7C%20Ubuntu%20%7C%20Debian-87cf3e.svg)](https://linuxmint.com)
[![Graphics: Vulkan 1.1](https://img.shields.io/badge/Graphics-Vulkan%201.1-red.svg)](https://www.vulkan.org/)
[![Window & Input: SDL3](https://img.shields.io/badge/Host-SDL3-blue.svg)](https://libsdl.org)
[![Recomp: XboxXBE-Recomp](https://img.shields.io/badge/Recomp-XboxXBE--Recomp-green.svg)](https://github.com/TGP482/XboxXBE-Recomp)
[![Status: 100+ FPS Verified](https://img.shields.io/badge/Performance-100%2B%20FPS%20Verified-success.svg)](#-performance--hardware)
[![Support: Saweria](https://img.shields.io/badge/Support-Saweria-orange.svg)](https://saweria.co/Kabako)
[![Support: Ko-fi](https://img.shields.io/badge/Support-Ko--fi-ff5e5b.svg)](https://ko-fi.com/katonbagaskoro)

Native Linux desktop build and `.deb` package of **Need for Speed: Most Wanted (Black Edition)** (2005 original Xbox release). Maintained by [Katon Bagaskoro (@Katon17)](https://github.com/Katon17), based on [RanggaCasper's MostWanted-Android](https://github.com/RanggaCasper/MostWanted-Android) and [TGP482's XboxXBE-Recomp](https://github.com/TGP482/XboxXBE-Recomp).

A native port gives you a single Vulkan binary that runs on Linux directly. That is pretty much the main benefit. Also we can fix original engine bugs in the C++ code instead of relying on memory hacks or DLL injection on Windows.

---

## 📸 Screenshots

| New Settings Icon | General Menu, Credits, and Exit Game |
|:---:|:---:|
| ![New Settings Icon](screenshots/new_settings_icon.png) | ![General Menu, Credits, and Exit Game](screenshots/general_menu_credits_exit.png) |

| Linux Options: Control Settings Removed | F10 Overlay: Keyboard & Controller Settings |
|:---:|:---:|
| ![Linux Options No Controls](screenshots/linux_options_no_controls.png) | ![F10 Overlay Keyboard](screenshots/f10_overlay_keyboard.png) |

| Gameplay Prologue Race (Locked 61 FPS) | Lenovo ThinkPad X270 Hardware Specs |
|:---:|:---:|
| ![Gameplay 60 FPS](screenshots/gameplay_60fps.png) | ![ThinkPad X270 Specs](screenshots/thinkpad_x270_specs.png) |

| Graphics: Yellow Tint Toggle | General Menu: Ko-fi & Saweria Support |
|:---:|:---:|
| ![Graphics Yellow Tint](screenshots/graphics_yellow_tint.png) | ![General Ko-fi Support](screenshots/general_kofi_support.png) |

---

## 🎮 Installation

Builds are available under releases on GitHub. You will need your own Xbox game ISO to build it. On Debian or Mint, install the deb file and launch the game. On Steam Deck, add the AppImage as a non-Steam game in desktop mode.

Check [README-STEAM-DECK.md](README-STEAM-DECK.md) for SteamOS details.

---

## 📊 Performance & Hardware

Tested and working on:
- Linux Mint
- Ubuntu
- Pop!_OS
- MX Linux
- Zorin OS
- Debian

Running native on an old ThinkPad X270 (HD 620) at 60 FPS without Wine.

---

## 🎮 Controls

The game uses WASD for driving and Spacebar for the handbrake. Standard gamepads are supported natively. If you want to change button bindings, press F10 while playing.

---

## 🛠️ Building from Source

```bash
git clone https://github.com/Katon17/nfsmw-linux.git
cd nfsmw-linux
./build-from-source.sh --deb
```

The resulting package will be generated at `release/nfsmw-linux_0.1.13_amd64.deb`.

---

## 🔧 Troubleshooting

Step 3/3 linking errors:
If you see undefined symbols during compilation, make sure you are on v0.1.13 or rebuild with:
```bash
nfsmw-linux --rebuild
```

Prologue cutscene:
You can skip the prologue cutscene with Spacebar or Enter. You can also clear shader cache with:
```bash
nfsmw-linux --clean-cache
```

---

## 📁 File Locations

Save games and mostwanted.toml are placed under `~/.local/share/nfsmw-linux`. You do not have to worry about updates overwriting your career progress.

---

## ☕ Support & Donations

* 💰 **Saweria**: [saweria.co/Kabako](https://saweria.co/Kabako)
* ☕ **Ko-fi**: [ko-fi.com/katonbagaskoro](https://ko-fi.com/katonbagaskoro)
* 📺 **YouTube**: [@KabakoNT](https://www.youtube.com/@KabakoNT)
* ⭐ **GitHub Star**: [Katon17/nfsmw-linux](https://github.com/Katon17/nfsmw-linux)

---

## 📜 Credits & Disclaimer

* **Linux Desktop Maintainer**: [Katon Bagaskoro (@Katon17)](https://github.com/Katon17)
* **Upstream Android Host Runtime**: [RanggaCasper/MostWanted-Android](https://github.com/RanggaCasper/MostWanted-Android)
* **Xbox Static Recompiler**: [TGP482/XboxXBE-Recomp](https://github.com/TGP482/XboxXBE-Recomp)
* **SpeedBreaker Project**: [SpeedBreakerProject/speedbreaker](https://github.com/SpeedBreakerProject/speedbreaker)
* **Libraries**: [SDL3](https://github.com/libsdl-org/SDL), [glslang](https://github.com/KhronosGroup/glslang), [Dear ImGui](https://github.com/ocornut/imgui), [toml++](https://github.com/marzer/tomlplusplus), [Zydis](https://github.com/zyantific/zydis).

Need for Speed is a trademark of Electronic Arts. I am not affiliated with EA, just made this for fun as an unofficial preservation project. You will need your own Xbox game ISO to build it.
