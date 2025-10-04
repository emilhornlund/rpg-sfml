#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || true)"
if [ -z "$REPO_ROOT" ]; then
  echo "Run this script from inside a Git repository."
  exit 1
fi

HOOKS_DIR="${REPO_ROOT}/.githooks"
mkdir -p "$HOOKS_DIR"

# Ensure our hook files are executable if they exist
for hook in pre-commit pre-push; do
  if [ -f "${HOOKS_DIR}/${hook}" ]; then
    chmod +x "${HOOKS_DIR}/${hook}"
  fi
done

# Point Git to use the repo-local hooks dir
git config core.hooksPath .githooks

echo "✔ Git hooks installed."
echo "   - core.hooksPath -> .githooks"
echo "   - Executable hooks: $(ls -1 ${HOOKS_DIR} | paste -sd ', ' -)"
