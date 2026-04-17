#!/usr/bin/env bash
# Format all C/C++/CUDA files with clang-format

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

echo "🎨 Formatting source files..."

find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.cu" -o -name "*.cuh" \) \
    ! -path "./build/*" \
    ! -path "./.git/*" \
    -exec clang-format -i {} +

echo "✅ Formatting complete!"
