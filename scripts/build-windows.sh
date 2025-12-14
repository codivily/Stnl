#!/usr/bin/env bash
set -euo pipefail

echo "Preparing Windows cross-build using CMake preset 'windows'"
if [ -z "${VCPKG_ROOT:-}" ]; then
  echo "ERROR: VCPKG_ROOT is not set. Install vcpkg and set VCPKG_ROOT to your vcpkg path."
  echo "Example:
  git clone https://github.com/microsoft/vcpkg.git $HOME/vcpkg
  cd $HOME/vcpkg
  ./bootstrap-vcpkg.sh
  export VCPKG_ROOT=$HOME/vcpkg"
  exit 1
fi

# Check common build prerequisites
if ! command -v ninja >/dev/null 2>&1; then
  echo "ERROR: 'ninja' is not installed. On Debian/Ubuntu: sudo apt-get install ninja-build"
  exit 1
fi

if ! command -v bison >/dev/null 2>&1; then
  echo "ERROR: 'bison' is not installed. On Debian/Ubuntu: sudo apt-get install bison"
  exit 1
fi

if ! command -v flex >/dev/null 2>&1; then
  echo "ERROR: 'flex' is not installed. On Debian/Ubuntu: sudo apt-get install flex"
  exit 1
fi

if ! command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1; then
  echo "WARNING: mingw cross-compiler 'x86_64-w64-mingw32-g++' not found in PATH."
  echo "Install it on Debian/Ubuntu: sudo apt-get install mingw-w64"
fi

echo "Removing stale build/windows to ensure a fresh configure..."
rm -rf build/windows

echo "Note: VCPKG_APPLOCAL_DEPS is disabled in the 'windows' preset to avoid running applocal.ps1 (requires PowerShell)."
echo "If you have PowerShell installed and want applocal behavior, set VCPKG_APPLOCAL_DEPS=ON in the preset or environment."
if command -v pwsh >/dev/null 2>&1 || command -v powershell.exe >/dev/null 2>&1; then
  echo "PowerShell detected; applocal could be enabled if desired."
else
  echo "PowerShell not detected: skipping applocal (safe for cross-build staging on Linux)."
fi

echo "Configuring (generator: Ninja) and building..."
cmake --preset windows
cmake --build build/windows -- -j$(nproc)

echo "Windows cross-build finished (output in build/windows)"
