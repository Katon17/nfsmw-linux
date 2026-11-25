# Need for Speed: Most Wanted (Black Edition) — Panduan Steam Deck & SteamOS

Panduan instalasi dan menjalankan **Need for Speed: Most Wanted (Black Edition)** secara native di **Valve Steam Deck (SteamOS)**.

---

## 🎮 Format Rilis untuk Steam Deck

Pada halaman [GitHub Releases](https://github.com/Katon17/nfsmw-linux/releases), kami menyediakan 2 format khusus yang kompatibel dengan Steam Deck:

1. **`nfsmw-linux_<version>_x86_64.AppImage`** *(Paling Direkomendasikan)*:
   Satu file biner executable langsung pakai.
2. **`nfsmw-linux_<version>_portable.tar.gz`**:
   Folder game mandiri tanpa instalasi root.

---

## 🚀 Cara Instalasi via AppImage (Paling Mudah)

### Langkah 1: Masuk ke Desktop Mode
1. Tekan tombol **STEAM** di Steam Deck.
2. Pilih menu **Power** -> **Switch to Desktop**.

### Langkah 2: Unduh & Beri Izin Eksekusi
1. Buka browser di Desktop Mode, unduh **`nfsmw-linux_0.1.13_x86_64.AppImage`** dari GitHub Releases.
2. Simpan di folder pilihan Anda (misalnya: `~/Games/` atau `~/Downloads/`).
3. Buka file manager **Dolphin**, klik kanan pada file AppImage -> **Properties** -> tab **Permissions** -> centang **"Is executable"** (atau via terminal: `chmod +x nfsmw-linux*.AppImage`).

### Langkah 3: Tambahkan ke Steam sebagai Non-Steam Game
1. Buka aplikasi **Steam** di Desktop Mode.
2. Klik menu **Games** di kiri atas -> **Add a Non-Steam Game to My Library...**
3. Klik **Browse...**, ubah filter file ke *All Files (*)*, lalu pilih file `nfsmw-linux_0.1.13_x86_64.AppImage`.
4. Klik **Add Selected Programs**.
5. Di Library Steam, klik kanan shortcut barunya -> **Properties**, ubah judul game menjadi:
   `Need for Speed: Most Wanted`

### Langkah 4: Jalankan Game & Pilih File ISO
1. Klik **Play** (bisa dilakukan di Desktop Mode atau Gaming Mode).
2. Pada peluncuran pertama, file picker akan muncul meminta Anda memilih file **ISO Xbox** original Anda.
3. Tunggu proses kompilasi satu kali (memakan waktu ~1–2 menit).
4. Selesai! Game langsung berjalan lancar **60 FPS** dengan kontroler bawaan Steam Deck yang 100% langsung terdeteksi.

---

## 📦 Cara Instalasi via Portable Tarball (`.tar.gz`)

Jika Anda lebih menyukai struktur folder portable:
```bash
# 1. Ekstrak tarball
mkdir -p ~/Games
tar -xzf nfsmw-linux_0.1.13_portable.tar.gz -C ~/Games/

# 2. Masuk ke direktori
cd ~/Games/nfsmw-linux-0.1.13-portable

# 3. Jalankan launcher
./nfsmw-linux
```
Lalu tambahkan file `~/Games/nfsmw-linux-0.1.13-portable/nfsmw-linux` ke Steam sebagai Non-Steam Game.

---

## 🔧 Pemecahan Masalah di Steam Deck (Troubleshooting)

### Error: "Compiler toolchain (clang++ lld) belum terpasang"
Pada SteamOS, compiler `clang` dan `lld` tidak terpasang secara bawaan. Ada dua cara mudah untuk mengatasinya:

#### Opsi 1: Salin dari PC (Paling Mudah — Tanpa Terminal di Steam Deck)
Jika Anda (atau teman Anda) sudah pernah menjalankan game ini sekali di laptop/PC Linux:
1. Buka folder `~/.local/share/nfsmw-linux/` di PC tersebut.
2. Salin seluruh folder `nfsmw-linux` tersebut ke Steam Deck di path yang sama (`/home/deck/.local/share/nfsmw-linux/`).
3. Jalankan AppImage di Steam Deck — game langsung jalan seketika tanpa perlu compile lagi!

#### Opsi 2: Instal Compiler via Terminal Konsole Steam Deck
Jika Anda ingin mengompilasi langsung di Steam Deck:
Buka **Konsole** di Desktop Mode, lalu jalankan perintah berikut:
```bash
sudo steamos-readonly disable
sudo pacman-key --init
sudo pacman-key --populate archlinux holo
sudo pacman -Sy --noconfirm clang lld python
sudo steamos-readonly enable
```
*(Catatan: Langkah `pacman-key` penting untuk mencegah pesan error "signature / corruption" di SteamOS).*
Setelah itu, buka AppImage kembali dan game akan langsung mengompilasi ISO Anda!

---

## ⚙️ Optimasi Performa & Baterai di Steam Deck
- **Framerate Target**: Game terkunci mulus di **60 FPS**.
- **TDP Limit**: Anda dapat menurunkan TDP Steam Deck ke **7W - 9W** di Quick Access Menu (`...` button) untuk menghemat baterai hingga 4–5 jam gameplay nonstop.
- **Kontroler**: Kontroler internal Steam Deck otomatis dikenali sebagai gamepad Xbox standard. Tekan tombol **F10** (atau petakan salah satu back paddle ke F10) jika ingin mengubah konfigurasi tombol.
