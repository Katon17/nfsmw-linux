#!/bin/bash
# publish-github.sh: Prepares and pushes nfsmw-linux to GitHub repository.
# Usage: ./publish-github.sh [<github_repo_url>]
set -euo pipefail
cd "$(dirname "$0")"

REMOTE_URL="${1:-}"

if [[ ! -d .git ]]; then
    echo "Initializing git repository..."
    git init -b main
fi

git config user.name "Katon17" || true
git config user.email "katon@example.invalid" || true

if [[ -n "$REMOTE_URL" ]]; then
    if git remote | grep -q origin; then
        git remote set-url origin "$REMOTE_URL"
    else
        git remote add origin "$REMOTE_URL"
    fi
    echo "Remote 'origin' set to: $REMOTE_URL"
fi

echo "Staging files..."
git add -A

echo "Checking status..."
git status -s

echo ""
echo "Creating initial commit..."
git commit -m "Initial commit: Native Linux desktop port and .deb package of Need for Speed: Most Wanted (Black Edition)" || true

echo ""
echo "======================================================================"
echo "Repository ready!"
if git remote | grep -q origin; then
    echo "To push to GitHub, run:"
    echo "  git push -u origin main"
else
    echo "To link your GitHub repo and push, run:"
    echo "  git remote add origin https://github.com/<your-username>/nfsmw-linux.git"
    echo "  git push -u origin main"
fi
echo ""
echo "To publish a Release with the pre-built .deb:"
echo "  gh release create v0.1.0 release/nfsmw-linux_0.1.0_amd64.deb --title \"nfsmw-linux v0.1.0\" --notes \"Native Linux port for Need for Speed: Most Wanted Black Edition\""
echo "======================================================================"
