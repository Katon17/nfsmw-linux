#!/bin/bash
# build-portable.sh: packages a portable directory and tarball for Steam Deck & generic Linux distributions.
# Usage: ./build-portable.sh [build dir] [out dir]
set -euo pipefail
cd "$(dirname "$0")"
REPO=$PWD
BUILD="${1:-$REPO/build}"
OUTDIR="${2:-$REPO/release}"
VERSION="$(cat VERSION | tr -d '[:space:]')"
MARCH="x86-64-v2"
STAGE="$BUILD/stage"

[[ -f $STAGE/lib/libnfsmw_host.a ]] || {
    echo "ERROR: No host library found in $STAGE. Build the 'stage' target first via:"
    echo "  cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && ninja -C build stage"
    exit 1
}

PORTABLE_DIR="$(mktemp -d)/nfsmw-linux-${VERSION}-portable"
mkdir -p "$PORTABLE_DIR/bin" "$PORTABLE_DIR/lib" "$PORTABLE_DIR/share/tools" "$PORTABLE_DIR/share/touch"

# 1. Launchers
for f in nfsmw-linux nfsmw-linux-build; do
    sed -e "s|@VERSION@|$VERSION|g" -e "s|@MARCH@|$MARCH|g" "packaging/$f" > "$PORTABLE_DIR/$f"
    chmod 755 "$PORTABLE_DIR/$f"
done

# 2. Recompiler binaries and host static libraries
install -m 755 -s "$STAGE/xbe_recomp" "$STAGE/xbe_symbols" "$PORTABLE_DIR/lib/"
install -m 644 "$STAGE/lib/libnfsmw_host.a" "$STAGE/lib/libSDL3.a" "$STAGE/lib/libglslang.a" \
               "$STAGE/lib/libglslang-default-resource-limits.a" "$STAGE/lib/libSPIRV.a" \
               "$STAGE/lib/libMachineIndependent.a" "$STAGE/lib/libGenericCodeGen.a" \
               "$STAGE/lib/libOSDependent.a" "$STAGE/image_symbols.h" "$PORTABLE_DIR/lib/"

printf '%s\n' libSDL3.a libglslang.a libglslang-default-resource-limits.a libSPIRV.a libMachineIndependent.a libGenericCodeGen.a libOSDependent.a > "$PORTABLE_DIR/lib/link-libs.txt"

# 3. Game configuration, SDK headers, and extraction tools
D="$PORTABLE_DIR/share"
install -D -m 644 config/game.toml "$D/config/game.toml"
(cd sdk && find runtime harness/compat -name '*.h' -exec install -D -m 644 {} "$D/sdk/{}" \;)
cp -r sdk/xdk "$D/sdk/xdk"
install -m 644 tools/xdvdfs.py "$D/tools/"

# 4. Touch assets & desktop metadata
if [[ -d assets/touch ]]; then
    cp -r assets/touch/* "$D/touch/"
fi
install -m 644 packaging/nfsmw-linux.desktop "$PORTABLE_DIR/"
install -m 644 packaging/nfsmw-linux.png "$PORTABLE_DIR/"

# 5. Steam Deck quick start guide
cat <<'EOF' > "$PORTABLE_DIR/README-STEAM-DECK.txt"
Need for Speed: Most Wanted (Black Edition) - Native Linux Portable
===================================================================

Panduan untuk Steam Deck / SteamOS:
-----------------------------------
1. Ekstrak arsip ini ke folder pilihan Anda, contoh:
   /home/deck/Games/nfsmw-linux/

2. Cara Menambahkan ke Steam (Desktop Mode):
   - Buka aplikasi Steam di Desktop Mode.
   - Klik menu "Games" di menu bar atas -> "Add a Non-Steam Game to My Library..."
   - Klik "Browse..." lalu pilih file executable "nfsmw-linux" di folder ini.
   - Klik "Add Selected Programs".

3. Menjalankan Game:
   - Kembali ke Steam Deck Gaming Mode.
   - Buka "Need for Speed: Most Wanted" di Library Anda.
   - Pada peluncuran pertama, jika game belum dikompilasi, game akan meminta Anda
     memilih file ISO game Xbox original Anda melalui jendela file browser.
   - Setelah selesai dikompilasi, game langsung berjalan lancar 60 FPS dengan
     kontroler bawaan Steam Deck yang langsung aktif otomatis!
EOF

mkdir -p "$OUTDIR"
ARCHIVE="$OUTDIR/nfsmw-linux_${VERSION}_portable.tar.gz"
echo "Membuat arsip portable: $ARCHIVE ..."
tar -C "$(dirname "$PORTABLE_DIR")" -czf "$ARCHIVE" "$(basename "$PORTABLE_DIR")"
rm -rf "$PORTABLE_DIR"

echo ""
echo "======================================================================"
echo "  SUCCESS: Built Portable package:"
echo "  $ARCHIVE"
echo "======================================================================"
