#!/usr/bin/env bash
# Format all C/C++/CUDA files with clang-format

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

# Prefer the same major version the CI format gate is pinned to, so local
# formatting and CI agree.
if command -v clang-format-18 >/dev/null 2>&1; then
    CLANG_FORMAT=clang-format-18
else
    CLANG_FORMAT=clang-format
fi

echo "🎨 Formatting source files with $CLANG_FORMAT..."

find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.cu" -o -name "*.cuh" \) \
    ! -path "./build/*" \
    ! -path "./.git/*" \
    -exec "$CLANG_FORMAT" -i {} +

echo "✅ Formatting complete!"
