#!/usr/bin/env bash

set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

echo "[INFO] Creating build directory..."
mkdir -p "${BUILD_DIR}"

echo "[INFO] Configuring project..."
cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}"

echo "[INFO] Building..."
cmake --build "${BUILD_DIR}" -j"$(nproc)"

echo "[INFO] Installing..."

if [ "$(id -u)" -eq 0 ]; then
    cmake --install "${BUILD_DIR}"
else
    sudo cmake --install "${BUILD_DIR}"
fi

echo
echo "[OK] Installation completed."
echo

which iv || true
