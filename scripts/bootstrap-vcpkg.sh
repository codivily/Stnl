#!/usr/bin/env bash
set -euo pipefail

VCPKG_DIR="${1:-${PWD}/vcpkg}"
if [ -d "$VCPKG_DIR" ]; then
  echo "vcpkg already present at $VCPKG_DIR"
else
  echo "Cloning vcpkg into $VCPKG_DIR"
  git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
fi

echo "Bootstrapping vcpkg (may take a few minutes)..."
pushd "$VCPKG_DIR" >/dev/null
if [ -f bootstrap-vcpkg.sh ]; then
  ./bootstrap-vcpkg.sh
else
  echo "Bootstrap script not found---this vcpkg version may be incompatible." >&2
  popd >/dev/null
  exit 1
fi
popd >/dev/null

echo "Bootstrapped vcpkg at $VCPKG_DIR"
echo "To use it for cross-builds, set: export VCPKG_ROOT=$VCPKG_DIR"
