#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"

echo "=== Packaging Trading Benchmark ==="

# Build first if needed
if [ ! -f "${BUILD_DIR}/risk_benchmark" ]; then
    echo "Building project first..."
    "${SCRIPT_DIR}/build.sh" Release
fi

cd "${BUILD_DIR}"

# Create packages using CPack
echo ""
echo "Creating distribution packages..."
cpack

echo ""
echo "=== Packaging Complete ==="
echo "Packages available in: ${BUILD_DIR}"
ls -la *.tar.gz *.zip *.deb *.rpm 2>/dev/null || true
