# Need for Speed: Most Wanted (Black Edition) — Port Native Linux

🇬🇧 **English version**: lihat [README.md](README.md)

[![Rilis: v0.1.13](https://img.shields.io/badge/Release-v0.1.13-blue.svg)](https://github.com/Katon17/nfsmw-linux/releases)
[![Platform: Linux x86_64](https://img.shields.io/badge/Platform-Linux%20x86__64-orange.svg)](https://kernel.org)
[![Target: Linux Mint / Ubuntu / Debian](https://img.shields.io/badge/Target-Linux%20Mint%20%7C%20Ubuntu%20%7C%20Debian-87cf3e.svg)](https://linuxmint.com)
[![Grafis: Vulkan 1.1](https://img.shields.io/badge/Graphics-Vulkan%201.1-red.svg)](https://www.vulkan.org/)
[![Jendela & Input: SDL3](https://img.shields.io/badge/Host-SDL3-blue.svg)](https://libsdl.org)
[![Rekompilasi: XboxXBE-Recomp](https://img.shields.io/badge/Recomp-XboxXBE--Recomp-green.svg)](https://github.com/TGP482/XboxXBE-Recomp)
[![Status: Terverifikasi 100+ FPS](https://img.shields.io/badge/Performance-100%2B%20FPS%20Verified-success.svg)](#-performa--spesifikasi-perangkat)
[![Dukungan: Saweria](https://img.shields.io/badge/Support-Saweria-orange.svg)](https://saweria.co/Kabako)
[![Dukungan: Ko-fi](https://img.shields.io/badge/Support-Ko--fi-ff5e5b.svg)](https://ko-fi.com/katonbagaskoro)

Build desktop native Linux dan paket `.deb` untuk **Need for Speed: Most Wanted (Black Edition)** (rilis Xbox original 2005). Dikelola oleh [Katon Bagaskoro (@Katon17)](https://github.com/Katon17), berbasis [MostWanted-Android milik RanggaCasper](https://github.com/RanggaCasper/MostWanted-Android) dan [XboxXBE-Recomp milik TGP482](https://github.com/TGP482/XboxXBE-Recomp).

Port native langsung menghasilkan satu biner Vulkan yang jalan di Linux tanpa emulator atau Wine. Itu kelebihan utamanya. Selain itu kita bisa langsung benerin bug engine game di kode C++ dibanding pakai memory hack di Windows.

---

## 📸 Tangkapan Layar

| New Settings Icon | General Menu, Credits, and Exit Game |
|:---:|:---:|
| ![New Settings Icon](screenshots/new_settings_icon.png) | ![General Menu, Credits, and Exit Game](screenshots/general_menu_credits_exit.png) |

| Linux Options: Pengaturan Kontrol Dihapus | Overlay F10: Pengaturan Keyboard & Controller |
|:---:|:---:|
| ![Linux Options Tanpa Kontrol](screenshots/linux_options_no_controls.png) | ![Overlay F10 Keyboard](screenshots/f10_overlay_keyboard.png) |

| Gameplay Balapan Prolog (Terkunci 61 FPS) | Spesifikasi Perangkat Lenovo ThinkPad X270 |
|:---:|:---:|
| ![Gameplay 60 FPS](screenshots/gameplay_60fps.png) | ![Spesifikasi ThinkPad X270](screenshots/thinkpad_x270_specs.png) |

| Grafis: Toggle Filter Kuning (Yellow Tint) | Menu General: Dukungan Donasi Ko-fi & Saweria |
|:---:|:---:|
| ![Grafis Yellow Tint](screenshots/graphics_yellow_tint.png) | ![Dukungan Donasi Ko-fi](screenshots/general_kofi_support.png) |

---

## 🎮 Cara Pasang

Paket instalasi ada di halaman releases GitHub. Kamu butuh file ISO game Xbox sendiri untuk memainkannya. Di Debian atau Mint, pasang file deb lalu jalankan gamenya. Di Steam Deck, tambahkan file AppImage sebagai non-Steam game di desktop mode.

Panduan lengkap SteamOS ada di [README-STEAM-DECK.md](README-STEAM-DECK.md).

---

## 📊 Performa & Spesifikasi Perangkat

Sudah dites jalan lancar di:
- Linux Mint
- Ubuntu
- Pop!_OS
- MX Linux
- Zorin OS
- Debian

Di ThinkPad X270 lawas (Intel HD 620), game jalan lancar di 60 FPS tanpa Wine.

---

## 🎮 Kontrol

Game ini pakai tombol WASD untuk stir dan gas, serta Spasi untuk rem tangan. Gamepad standar langsung kedeteksi otomatis. Kalau mau ganti konfigurasi tombol, tinggal tekan F10 pas game jalan.

---

## 🛠️ Kompilasi dari Source

```bash
git clone https://github.com/Katon17/nfsmw-linux.git
cd nfsmw-linux
./build-from-source.sh --deb
```

Paket `.deb` hasil kompilasi akan tersimpan di `release/nfsmw-linux_0.1.13_amd64.deb`.

---

## 🔧 Pemecahan Masalah (Troubleshooting)

Error linking di Step 3/3:
Kalau muncul undefined symbol saat compile, pastikan pakai versi v0.1.13 atau rebuild dengan:
```bash
nfsmw-linux --rebuild
```

Cutscene prolog:
Kamu bisa lewati video prolog pakai tombol Spacebar atau Enter. Kalau mau bersihkan cache shader:
```bash
nfsmw-linux --clean-cache
```

---

## 📁 Lokasi Berkas

File save dan mostwanted.toml tersimpan di `~/.local/share/nfsmw-linux`. Jadi update versi tidak akan menimpa progress career kamu.

---

## ☕ Donasi & Dukungan

* 💰 **Saweria**: [saweria.co/Kabako](https://saweria.co/Kabako)
* ☕ **Ko-fi**: [ko-fi.com/katonbagaskoro](https://ko-fi.com/katonbagaskoro)
* 📺 **YouTube**: [@KabakoNT](https://www.youtube.com/@KabakoNT)
* ⭐ **GitHub Star**: [Katon17/nfsmw-linux](https://github.com/Katon17/nfsmw-linux)

---

## 📜 Kredit & Lisensi

* **Maintainer Desktop Linux**: [Katon Bagaskoro (@Katon17)](https://github.com/Katon17)
* **Host Runtime Android Upstream**: [RanggaCasper/MostWanted-Android](https://github.com/RanggaCasper/MostWanted-Android)
* **Static Recompiler Xbox**: [TGP482/XboxXBE-Recomp](https://github.com/TGP482/XboxXBE-Recomp)
* **SpeedBreaker Project**: [SpeedBreakerProject/speedbreaker](https://github.com/SpeedBreakerProject/speedbreaker)
* **Pustaka**: [SDL3](https://github.com/libsdl-org/SDL), [glslang](https://github.com/KhronosGroup/glslang), [Dear ImGui](https://github.com/ocornut/imgui), [toml++](https://github.com/marzer/tomlplusplus), [Zydis](https://github.com/zyantific/zydis).

Need for Speed adalah merek dagang Electronic Arts. Saya tidak terafiliasi dengan EA, proyek ini dibuat untuk kesenangan dan preservasi game. Kamu butuh file ISO Xbox sendiri untuk memainkannya.
