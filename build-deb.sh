#!/bin/bash
# build-deb.sh: creates the nfsmw-linux .deb package from the finished host build.
# Target: Linux Mint 22.x / Ubuntu 24.04 LTS (x86_64).
# Usage: ./build-deb.sh [build dir] [out dir]
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
    echo "  cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && ninja -C build"
    exit 1
}

PKG="$(mktemp -d)/nfsmw-linux_${VERSION}_amd64"
mkdir -p "$PKG"/DEBIAN "$PKG"/usr/bin "$PKG"/usr/lib/nfsmw-linux "$PKG"/usr/share/nfsmw-linux/tools \
         "$PKG"/usr/share/nfsmw-linux/touch "$PKG"/usr/share/applications \
         "$PKG"/usr/share/icons/hicolor/scalable/apps "$PKG"/usr/share/doc/nfsmw-linux

# 1. Launchers
for f in nfsmw-linux nfsmw-linux-build; do
    sed -e "s|@VERSION@|$VERSION|g" -e "s|@MARCH@|$MARCH|g" "packaging/$f" > "$PKG/usr/bin/$f"
    chmod 755 "$PKG/usr/bin/$f"
done

# 2. Recompiler binaries and host static libraries
install -m 755 -s "$STAGE/xbe_recomp" "$STAGE/xbe_symbols" "$PKG/usr/lib/nfsmw-linux/"
install -m 644 "$STAGE/lib/libnfsmw_host.a" "$STAGE/lib/libSDL3.a" "$STAGE/lib/libglslang.a" \
               "$STAGE/lib/libglslang-default-resource-limits.a" "$STAGE/lib/libSPIRV.a" \
               "$STAGE/lib/libMachineIndependent.a" "$STAGE/lib/libGenericCodeGen.a" \
               "$STAGE/lib/libOSDependent.a" "$STAGE/image_symbols.h" "$PKG/usr/lib/nfsmw-linux/"

printf '%s\n' libSDL3.a libglslang.a libglslang-default-resource-limits.a libSPIRV.a libMachineIndependent.a libGenericCodeGen.a libOSDependent.a > "$PKG/usr/lib/nfsmw-linux/link-libs.txt"

# 3. Game configuration, SDK headers, and extraction tools
D="$PKG/usr/share/nfsmw-linux"
install -D -m 644 config/game.toml "$D/config/game.toml"
(cd sdk && find runtime harness/compat -name '*.h' -exec install -D -m 644 {} "$D/sdk/{}" \;)
cp -r sdk/xdk "$D/sdk/xdk"
install -m 644 tools/xdvdfs.py "$D/tools/"

# 4. Touch assets
if [[ -d assets/touch ]]; then
    cp -r assets/touch/* "$D/touch/"
fi

# 5. Desktop shortcuts and application icon
install -m 644 packaging/nfsmw-linux.desktop "$PKG/usr/share/applications/"
install -m 644 packaging/nfsmw-linux.svg "$PKG/usr/share/icons/hicolor/scalable/apps/"

# 6. Documentation and licenses
install -m 644 packaging/README.Debian "$PKG/usr/share/doc/nfsmw-linux/README"
{
    echo "nfsmw-linux: Native Linux port of Need for Speed: Most Wanted Black Edition"
    echo "Based on RanggaCasper/MostWanted-Android and XboxXBE-Recomp (TGP482)."
    echo "This package contains NO game code, binaries, or EA proprietary assets."
    echo "Need for Speed: Most Wanted is © Electronic Arts / EA Black Box."
    echo ""
    echo "== MostWanted-Android Host Runtime =="; cat LICENSE 2>/dev/null || echo "MIT License"; echo
    echo "== XboxXBE-Recomp SDK =="; cat sdk/LICENSE 2>/dev/null || echo "MIT License"; echo
    echo "== Third Party Libraries =="
    echo "SDL3: zlib license (c) Sam Lantinga / libsdl.org"
    echo "glslang: BSD-3-Clause / Apache-2.0 (c) Khronos Group"
    echo "Dear ImGui: MIT License (c) Omar Cornut"
    echo "toml++: MIT License (c) Mark Gillard"
    echo "Zydis: MIT License (c) Florian Bernd"
} > "$PKG/usr/share/doc/nfsmw-linux/copyright"

SIZE=$(du -sk "$PKG/usr" | cut -f1)
cat > "$PKG/DEBIAN/control" <<EOF
Package: nfsmw-linux
Version: $VERSION
Architecture: amd64
Maintainer: nfsmw-linux packager <noreply@example.invalid>
Installed-Size: $SIZE
Depends: libc6 (>= 2.38), libstdc++6, libgcc-s1, libvulkan1, mesa-vulkan-drivers | vulkan-icd, clang, lld, libc6-dev, libstdc++-14-dev | libstdc++-13-dev | libstdc++-dev, python3, zenity
Recommends: vulkan-tools
Section: games
Priority: optional
Homepage: https://github.com/RanggaCasper/MostWanted-Android
Description: Need for Speed: Most Wanted Black Edition as a native Linux game
 An unofficial native Linux (x86_64) port of the original Xbox version of
 Need for Speed: Most Wanted Black Edition (2005), created via static recompilation
 (XboxXBE-Recomp).
 .
 Features:
  • Pure native x86_64 ELF binary — no emulator (Xemu/RPCS3), no Wine, no Proton
  • Modern Vulkan 1.1 renderer with Intel HD 620 / low-spec optimization
  • Native SDL3 display, low-latency audio, gamepad and keyboard/mouse input
  • Integrated Xbox virtual memory, disc file system, and D3D8 translation
 .
 The package contains NO copyrighted EA game binaries or assets. On first run,
 it recompiles default.xbe from YOUR OWN legally owned Xbox game copy
 (.iso disc image or extracted files) into ~/.local/share/nfsmw-linux/bin/MOSTWANTED.
EOF

cat > "$PKG/DEBIAN/postinst" <<'EOF'
#!/bin/sh
set -e
command -v gtk-update-icon-cache >/dev/null 2>&1 && gtk-update-icon-cache -q -t -f /usr/share/icons/hicolor || true
command -v update-desktop-database >/dev/null 2>&1 && update-desktop-database -q /usr/share/applications || true
exit 0
EOF
chmod 755 "$PKG/DEBIAN/postinst"

cat > "$PKG/DEBIAN/postrm" <<'EOF'
#!/bin/sh
set -e
if [ "$1" = "purge" ]; then
    echo "nfsmw-linux: User save files and built binary remain in ~/.local/share/nfsmw-linux"
fi
exit 0
EOF
chmod 755 "$PKG/DEBIAN/postrm"

mkdir -p "$OUTDIR"
find "$PKG" -type d -exec chmod 755 {} \;
fakeroot dpkg-deb --build -Zxz "$PKG" "$OUTDIR/nfsmw-linux_${VERSION}_amd64.deb"
rm -rf "$(dirname "$PKG")"

echo ""
echo "======================================================================"
echo "  SUCCESS: Built Debian package:"
echo "  $OUTDIR/nfsmw-linux_${VERSION}_amd64.deb"
echo "======================================================================"
