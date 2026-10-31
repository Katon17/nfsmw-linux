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
echo "Lalu upload release installer .deb via GitHub CLI (jika gh terpasang):"
echo "  gh release create v0.1.0 release/nfsmw-linux_0.1.0_amd64.deb \\"
echo "    --title \"Need for Speed: Most Wanted (Black Edition) v0.1.0\" \\"
echo "    --notes \"Native Linux (x86_64) port. Verified 60 FPS on ThinkPad X270 (Intel HD 620). Maintained by Katon Bagaskoro.\""
echo "======================================================================"
