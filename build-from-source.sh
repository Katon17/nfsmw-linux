#!/bin/bash
# build-from-source.sh: Builds the entire native Linux port (host runtime + recompiler tools)
# from source, and optionally produces a playable binary if a game ISO is provided.
#
# Usage:
#   ./build-from-source.sh [--deb] [<path to game .iso or folder>]
set -euo pipefail
cd "$(dirname "$0")"
REPO=$PWD

BUILD_DEB=0
GAME_PATH=""
for arg in "$@"; do
    case "$arg" in
        --deb) BUILD_DEB=1 ;;
        *) GAME_PATH="$arg" ;;
    esac
done

echo "======================================================================"
echo "  Building Need for Speed: Most Wanted (Black Edition) for Linux"
echo "======================================================================"

# 1. Dependency check
MISSING=()
for pkg in cmake ninja clang lld python3; do
    command -v "$pkg" >/dev/null 2>&1 || MISSING+=("$pkg")
done

if [[ ${#MISSING[@]} -gt 0 ]]; then
    echo "Missing build tools: ${MISSING[*]}"
    if command -v apt-get >/dev/null 2>&1; then
        echo "Attempting to install dependencies via apt..."
        sudo apt-get update && sudo apt-get install -y \
            build-essential cmake ninja-build clang lld python3 \
            libvulkan-dev libasound2-dev libpulse-dev libx11-dev \
            libxcursor-dev libxrandr-dev libxi-dev libxinerama-dev \
            libxkbcommon-dev libxtst-dev libxxf86vm-dev libwayland-dev
    else
        echo "Please install the missing tools using your system package manager."
        exit 1
    fi
fi

# 2. Configure & Build
mkdir -p "$REPO/build"
cmake -B "$REPO/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C "$REPO/build"

echo ""
echo "Host runtime and recompiler tools built successfully in build/stage/!"
echo ""

# 3. Build .deb if requested
if [[ $BUILD_DEB -eq 1 ]]; then
    echo "Creating Debian package (.deb)..."
    ./build-deb.sh "$REPO/build" "$REPO/release"
fi

# 4. If game path is supplied, compile game binary
if [[ -n "$GAME_PATH" ]]; then
    echo "Compiling game from provided source: $GAME_PATH"
    export NFSMW_DATA_DIR="$REPO"
    export NFSMW_LIB_DIR="$REPO/build/stage"
    packaging/nfsmw-linux-build "$GAME_PATH"
fi
