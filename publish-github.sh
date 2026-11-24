#!/bin/bash
# publish-github.sh: Prepares and pushes nfsmw-linux to https://github.com/Katon17/nfsmw-linux
set -euo pipefail
cd "$(dirname "$0")"

REPO_URL="https://github.com/Katon17/nfsmw-linux.git"

if [[ ! -d .git ]]; then
    echo "Initializing git repository..."
    git init -b main
fi

git config user.name "Katon Bagaskoro"
git config user.email "katon@thinkpad.local"

VERSION="$(cat VERSION | tr -d '[:space:]')"

if ! git rev-parse --verify HEAD >/dev/null 2>&1; then
    echo "Creating initial commit..."
    git add .
    git commit -m "feat: initial release v${VERSION} of Need for Speed: Most Wanted (Black Edition) native Linux port"
elif [[ -n $(git status --porcelain) ]]; then
    echo "Committing pending changes..."
    git add .
    git commit -m "update: latest fixes and enhancements v${VERSION}"
fi

if git remote | grep -q origin; then
    git remote set-url origin "$REPO_URL"
else
    git remote add origin "$REPO_URL"
fi
echo "Remote 'origin' set to: $REPO_URL"

echo ""
echo "======================================================================"
echo "  Repository is completely configured and ready to push to GitHub!"
echo "======================================================================"
echo ""
echo "Jalankan perintah ini untuk push ke GitHub:"
echo "  git push -u origin main"
echo ""
echo "Lalu upload file rilis (.deb, .AppImage, portable .tar.gz) via GitHub CLI (jika gh terpasang):"
echo "  gh release create v${VERSION} \\"
echo "    release/nfsmw-linux_${VERSION}_amd64.deb \\"
echo "    release/nfsmw-linux_${VERSION}_x86_64.AppImage \\"
echo "    release/nfsmw-linux_${VERSION}_portable.tar.gz \\"
echo "    --title \"Need for Speed: Most Wanted (Black Edition) v${VERSION}\" \\"
echo "    --notes \"Native Linux (x86_64) port. Added Steam Deck support with standalone AppImage and Portable tarball (.tar.gz), verified crystal-clear audio with automatic PulseAudio driver configuration, eliminating low-quantum buffer underruns. Maintained by Katon Bagaskoro.\""
echo "======================================================================"
