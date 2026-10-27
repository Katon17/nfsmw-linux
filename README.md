# Need for Speed: Most Wanted (Black Edition) — Native Linux Port

[![Platform: Linux x86_64](https://img.shields.io/badge/Platform-Linux%20x86__64-orange.svg)](https://kernel.org)
[![Target: Linux Mint / Ubuntu](https://img.shields.io/badge/Target-Linux%20Mint%2022%20%7C%20Ubuntu%2024.04-87cf3e.svg)](https://linuxmint.com)
[![Graphics: Vulkan 1.1](https://img.shields.io/badge/Graphics-Vulkan%201.1-red.svg)](https://www.vulkan.org/)
[![Window & Input: SDL3](https://img.shields.io/badge/Host-SDL3-blue.svg)](https://libsdl.org)
[![Recomp: XboxXBE-Recomp](https://img.shields.io/badge/Recomp-XboxXBE--Recomp-green.svg)](https://github.com/TGP482/XboxXBE-Recomp)

A native Linux desktop port and `.deb` packaging of **Need for Speed: Most Wanted (Black Edition)** (2005, original Xbox), based on [RanggaCasper's MostWanted-Android](https://github.com/RanggaCasper/MostWanted-Android) and [TGP482's XboxXBE-Recomp](https://github.com/TGP482/XboxXBE-Recomp).

---

## ⚡ Why This Port?

Traditional solutions for playing older console games on Linux suffer from major bottlenecks:
- **Virtualization & Emulators (Xemu, RPCS3)**: Emulate hardware CPU instructions, guest MMU, GPU interrupts, and sound processors in software loops. On dual-core ultrabooks like the **Lenovo ThinkPad X270 (Intel Core i5 7th Gen, Intel HD Graphics 620)**, emulators choke down to 12–20 FPS with audio stutters.
- **Wine / Proton Wrappers**: Incur DXVK API translation layers, Windows ABI shims, and heavy memory footprints.

**`nfsmw-linux` takes a completely different, pure native approach**:
1. **Static Recompilation**: Translates original Xbox x86 machine instructions from `default.xbe` directly into modern C++ source code ahead of time.
2. **Native Host Runtime**: The game's Direct3D 8 calls, DirectSound streams, and kernel thread waits are translated directly into high-performance **Vulkan 1.1** draw calls, native **SDL3 audio/events**, and Linux POSIX threads.
3. **No Emulation Layer**: Compiles to an optimized native x86_64 ELF binary that executes directly on bare metal CPU and GPU.
4. **Fluid 60 FPS**: Runs butter-smooth at native display resolution even on 15W Intel HD Graphics 620 integrated GPUs.

---

## 📦 Features

- **Pure Native Linux ELF (x86_64)**: Zero Wine, zero Proton, zero emulation overhead.
- **Vulkan 1.1 Renderer**: Custom high-throughput GPU backend supporting bloom, distance fog, reflections, rain effects, and custom resolutions.
- **SDL3 Desktop Integration**: Native Wayland and X11 display support, PulseAudio / PipeWire / ALSA low-latency sound, and raw gamepad input.
- **Full Controller & Keyboard Support**: Hot-plugging for Xbox One/Series, DualShock 4, DualSense, and generic gamepads. Complete keyboard & mouse steering/navigation.
- **One-Command Build & Packaging**: Fully automated `.deb` installer generation and one-click build tool (`nfsmw-linux-build`).
- **Clean Game Asset Separation**: Contains zero copyrighted EA code or art. Uses your legally owned game copy (`.iso` disc image or extracted files).

---

## 🖥️ System Requirements

| Component | Minimum | Recommended |
|---|---|---|
| **OS** | Linux Mint 22.x / Ubuntu 24.04 LTS (or modern Linux distro) | Linux Mint 22.3 XFCE / Ubuntu 24.04 LTS |
| **CPU** | Intel Core i3 / i5 4th Gen dual-core | Intel Core i5 7th Gen (e.g. ThinkPad X270) or higher |
| **GPU** | Intel HD Graphics 520 / 620, AMD Radeon, or NVIDIA (Vulkan 1.1 support) | Intel HD Graphics 620 or discrete GPU |
| **RAM** | 4 GB | 8 GB (for faster C++ parallel compilation) |
| **Storage** | 3.5 GB available space | SSD storage recommended |

---

## 🚀 Quick Start (Debian / Linux Mint / Ubuntu)

### Step 1: Install the `.deb` Package

Download or build `nfsmw-linux_0.1.0_amd64.deb` and install it:

```bash
sudo apt install ./release/nfsmw-linux_0.1.0_amd64.deb
```

This installs:
- Launcher command: `/usr/bin/nfsmw-linux`
- Builder utility: `/usr/bin/nfsmw-linux-build`
- Host runtime & recompiler: `/usr/lib/nfsmw-linux/`
- SDK files & configs: `/usr/share/nfsmw-linux/`
- Application shortcut & icon in your desktop menu under **Games**

### Step 2: Supply Your Game Copy

Place your legally owned copy of **Need for Speed: Most Wanted Black Edition (original Xbox)** into:

```bash
mkdir -p ~/.local/share/nfsmw-linux/game
cp /path/to/your/NFSMW_Black_Edition.iso ~/.local/share/nfsmw-linux/game/
```

*(Extracted game folders containing `default.xbe` are also supported).*

### Step 3: Build & Play

Launch from your desktop application menu or terminal:

```bash
nfsmw-linux
```

On the very first launch, `nfsmw-linux-build` will:
1. Automatically extract `default.xbe` from your ISO disc image using built-in XDVDFS.
2. Recompile the game code to C++ using `xbe_recomp`.
3. Compile and link the native binary into `~/.local/share/nfsmw-linux/bin/MOSTWANTED`.

Subsequent launches bypass the builder and boot directly into the game in less than 2 seconds!

---

## 🛠️ Building from Source

To compile the entire host runtime, recompiler tools, and Debian package from scratch:

```bash
# 1. Clone this repository
git clone https://github.com/<your-username>/nfsmw-linux.git
cd nfsmw-linux

# 2. Build host and tools
./build-from-source.sh --deb
```

The script will automatically detect missing dependencies, configure CMake + Ninja, build the `stage` host library, and generate `release/nfsmw-linux_0.1.0_amd64.deb`.

---

## 🎮 Controls

### Gamepad (Xbox, PlayStation, Generic)
- **Throttle / Accelerate**: Right Trigger (`RT` / `R2`)
- **Brake / Reverse**: Left Trigger (`LT` / `L2`)
- **Steering**: Left Thumbstick (`LX`)
- **Handbrake**: `A` (Xbox) / `Cross` (PlayStation)
- **Nitrous (NOS)**: `X` (Xbox) / `Square` (PlayStation)
- **SpeedBreaker**: `B` (Xbox) / `Circle` (PlayStation)
- **Shift Up / Down**: Right Bumper (`RB`) / Left Bumper (`LB`) (Manual transmission)
- **Look Back**: Right Stick Click (`RS`) / `Y` (Xbox) / `Triangle` (PlayStation)
- **Pause Menu**: `Start`

### Keyboard
- **Accelerate**: `W` or `Up Arrow`
- **Brake / Reverse**: `S` or `Down Arrow`
- **Steer Left / Right**: `A` / `D` or `Left` / `Right Arrow`
- **Handbrake**: `Spacebar`
- **Nitrous**: `Left Shift` or `Right Shift`
- **SpeedBreaker**: `Left Alt` or `Keypad 0`
- **Look Back**: `C` or `Middle Mouse Button`
- **Pause / In-Game Menu**: `Escape` or `Enter`

---

## ⚙️ Configuration & Customization

All player configuration is stored in human-readable TOML at:
`~/.local/share/nfsmw-linux/settings.toml`

### Display & Graphics
- `display_mode`: `"fullscreen"`, `"borderless"`, or `"windowed"`.
- `width` / `height`: Target render resolution.
- `anti_aliasing`: `0` (Off), `1` (FXAA), or `2` (SSAA 2x).
- `bloom`: Enable or disable cinematic sun glare / bloom glow.
- `fog`: Enable or disable distance atmospheric fog.
- `car_reflections`: Real-time car paint cubemap reflection rate (`0` = full 60fps, `1` = 30fps, `2` = 15fps, `3` = off).
- `road_reflections`: Dynamic wet asphalt screen reflections.

### Saved Profiles & Memory Cards
Xbox virtual storage (`C:` and `E:` partitions) is located at:
`~/.local/share/nfsmw-linux/hdd/`
Profiles, career progression, Blacklist rankings, and tuned car garages persist here safely across updates.

---

## 📜 Credits & License

- **MostWanted-Android**: Host runtime and decompilation analysis by [RanggaCasper](https://github.com/RanggaCasper/MostWanted-Android).
- **XboxXBE-Recomp**: Xbox executable recompiler and runtime SDK by [TGP482](https://github.com/TGP482/XboxXBE-Recomp).
- **SpeedBreaker**: Linux x86_64 Most Wanted research by [SpeedBreakerProject](https://github.com/SpeedBreakerProject/speedbreaker).
- **SDL3**: Simple DirectMedia Layer by [libsdl-org](https://github.com/libsdl-org/SDL).
- **glslang**: Khronos Group GLSL/HLSL shading language compiler.
- **Dear ImGui**: Immediate mode graphical user interface by Omar Cornut.

*Disclaimer: Need for Speed™ and Need for Speed: Most Wanted are registered trademarks of Electronic Arts Inc. This project is an unofficial community project with no affiliation to EA, Criterion Games, or Microsoft. No copyrighted assets or executables are distributed.*
