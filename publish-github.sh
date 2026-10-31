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

if ! git rev-parse --verify HEAD >/dev/null 2>&1; then
    echo "Creating initial commit..."
    git add .
    git commit -m "feat: initial release v0.1.4 of Need for Speed: Most Wanted (Black Edition) native Linux port"
elif [[ -n $(git status --porcelain) ]]; then
    echo "Committing pending changes..."
    git add .
    git commit -m "update: latest fixes and enhancements v0.1.4"
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
echo "Lalu upload release installer .deb via GitHub CLI (jika gh terpasang):"
echo "  gh release create v0.1.4 release/nfsmw-linux_0.1.4_amd64.deb \\"
echo "    --title \"Need for Speed: Most Wanted (Black Edition) v0.1.4\" \\"
echo "    --notes \"Native Linux (x86_64) port. Linux Options UI with Tux icon, verified 100+ FPS on ThinkPad X270 (Intel HD 620). Maintained by Katon Bagaskoro.\""
echo "======================================================================"
