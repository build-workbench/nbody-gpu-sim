#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"
git config core.hooksPath .githooks
# git silently skips hooks without the executable bit; make sure they are set.
chmod +x .githooks/*

echo "✅ Git hooks enabled via .githooks/"
