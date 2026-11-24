#!/bin/bash
# build-appimage.sh: builds a standalone x86_64 AppImage for Steam Deck & universal Linux.
# Usage: ./build-appimage.sh [build dir] [out dir]
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

# Ensure appimagetool is ready in /tmp
APPIMAGETOOL="/tmp/appimagetool"
if [[ ! -x $APPIMAGETOOL ]]; then
    echo "Mengunduh appimagetool ke /tmp..."
    curl -sSL -o "$APPIMAGETOOL" "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
    chmod +x "$APPIMAGETOOL"
fi

APPDIR="$(mktemp -d)/nfsmw-linux.AppDir"
mkdir -p "$APPDIR"/usr/bin "$APPDIR"/usr/lib/nfsmw-linux "$APPDIR"/usr/share/nfsmw-linux/tools \
         "$APPDIR"/usr/share/nfsmw-linux/touch "$APPDIR"/usr/share/applications \
         "$APPDIR"/usr/share/icons/hicolor/scalable/apps "$APPDIR"/usr/share/pixmaps

# 1. Launchers
for f in nfsmw-linux nfsmw-linux-build; do
    sed -e "s|@VERSION@|$VERSION|g" -e "s|@MARCH@|$MARCH|g" "packaging/$f" > "$APPDIR/usr/bin/$f"
    chmod 755 "$APPDIR/usr/bin/$f"
done

# 2. Recompiler binaries and host static libraries
install -m 755 -s "$STAGE/xbe_recomp" "$STAGE/xbe_symbols" "$APPDIR/usr/lib/nfsmw-linux/"
install -m 644 "$STAGE/lib/libnfsmw_host.a" "$STAGE/lib/libSDL3.a" "$STAGE/lib/libglslang.a" \
               "$STAGE/lib/libglslang-default-resource-limits.a" "$STAGE/lib/libSPIRV.a" \
               "$STAGE/lib/libMachineIndependent.a" "$STAGE/lib/libGenericCodeGen.a" \
               "$STAGE/lib/libOSDependent.a" "$STAGE/image_symbols.h" "$APPDIR/usr/lib/nfsmw-linux/"

printf '%s\n' libSDL3.a libglslang.a libglslang-default-resource-limits.a libSPIRV.a libMachineIndependent.a libGenericCodeGen.a libOSDependent.a > "$APPDIR/usr/lib/nfsmw-linux/link-libs.txt"

# 3. Game configuration, SDK headers, and extraction tools
D="$APPDIR/usr/share/nfsmw-linux"
install -D -m 644 config/game.toml "$D/config/game.toml"
(cd sdk && find runtime harness/compat -name '*.h' -exec install -D -m 644 {} "$D/sdk/{}" \;)
cp -r sdk/xdk "$D/sdk/xdk"
install -m 644 tools/xdvdfs.py "$D/tools/"

# 4. Touch assets
if [[ -d assets/touch ]]; then
    cp -r assets/touch/* "$D/touch/"
fi

# 5. Desktop file, icons, and AppRun
install -m 644 packaging/nfsmw-linux.desktop "$APPDIR/"
install -m 644 packaging/nfsmw-linux.desktop "$APPDIR/usr/share/applications/"
install -m 644 packaging/nfsmw-linux.png "$APPDIR/"
install -m 644 packaging/nfsmw-linux.png "$APPDIR/usr/share/pixmaps/"

cat <<'EOF' > "$APPDIR/AppRun"
#!/bin/bash
HERE="$(dirname "$(readlink -f "${0}")")"
export APPDIR="$HERE"
exec "$HERE/usr/bin/nfsmw-linux" "$@"
EOF
chmod 755 "$APPDIR/AppRun"

mkdir -p "$OUTDIR"
OUTPUT_APPIMAGE="$OUTDIR/nfsmw-linux_${VERSION}_x86_64.AppImage"
echo "Membangun AppImage: $OUTPUT_APPIMAGE ..."
ARCH=x86_64 "$APPIMAGETOOL" --appimage-extract-and-run "$APPDIR" "$OUTPUT_APPIMAGE"
rm -rf "$APPDIR"

echo ""
echo "======================================================================"
echo "  SUCCESS: Built AppImage:"
echo "  $OUTPUT_APPIMAGE"
echo "======================================================================"
