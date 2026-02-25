#!/bin/bash
set -e

BUILD_DIR_LINUX="./build_linux"
BUILD_DIR_WIN="./build_win"
BUILD_FOR_WINDOWS=0
BUILD_ALL=0
CLEAN_BUILD=0

# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    --clean|-c)
      CLEAN_BUILD=1
      ;;
    --windows|-w)
      BUILD_FOR_WINDOWS=1
      ;;
    --all|-a)
      BUILD_ALL=1
      ;;
    --help)
      echo "Usage: build.sh [--clean|-c] [--windows|-w] [--all|-a] [--help]"
exit 0
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
  shift
done

# Handle clean build
if [[ $CLEAN_BUILD -eq 1 ]]; then
  echo "Deleting Build Directories"
  rm -rf "$BUILD_DIR_LINUX" "$BUILD_DIR_WIN"
fi

# Build for Windows
if [[ $BUILD_FOR_WINDOWS -eq 1 || $BUILD_ALL -eq 1 ]]; then
  echo "Configuring Windows build..."
  cmake -B"$BUILD_DIR_WIN" -H. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_C_COMPILER="x86_64-w64-mingw32-gcc-posix" \
    -DCMAKE_CXX_COMPILER="x86_64-w64-mingw32-g++-posix"
  make -C"$BUILD_DIR_WIN" -j6
fi

# Build for Linux
echo "Configuring Linux build..."
cmake -B"$BUILD_DIR_LINUX" -H. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20
make -C"$BUILD_DIR_LINUX" -j6
