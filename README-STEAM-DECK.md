# Need for Speed Most Wanted — Panduan Steam Deck

Format Rilis:
- AppImage: File executable tunggal, tinggal tambah ke Steam.
- Portable tar.gz: Ekstrak ke folder Games tanpa perlu root.

Cara Pasang:
1. Masuk ke Desktop Mode.
2. Download file AppImage dari halaman Releases dan beri izin eksekusi.
3. Di Steam, klik Games -> Add a Non-Steam Game, pilih file AppImage.
4. Buka game lewat Gaming Mode, lalu pilih file ISO Xbox kamu.

Kalau pacman error saat compile di Steam Deck:
Valve mengunci filesystem SteamOS dan membiarkan Arch keyring kosong. Buka Konsole di Desktop Mode dan jalankan:
```bash
sudo steamos-readonly disable
sudo pacman-key --init
sudo pacman-key --populate archlinux holo
sudo pacman -Sy --noconfirm clang lld python
sudo steamos-readonly enable
```
Setelah itu buka lagi AppImage-nya.

Kalau malas install compiler di Steam Deck, kamu bisa copy folder `~/.local/share/nfsmw-linux/` dari PC Linux yang sudah pernah dipakai main ke `/home/deck/.local/share/nfsmw-linux/`.

Performa & Kontrol:
Game jalan di 60 FPS. Tombol bawaan Steam Deck otomatis kebaca. Kalau mau hemat baterai bisa set TDP ke 8W atau 9W di menu Quick Access.
