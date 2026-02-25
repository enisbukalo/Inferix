#!/bin/bash
# build.sh - Build Inferix with parallel builds using CMake

set -e

cd "$PWD"

build_dir="build"

# Create build directory if it doesn't exist
if [ ! -d "$build_dir" ]; then
  echo "Creating build directory: $build_dir"
  mkdir -p "$build_dir"
fi

echo "Configuring build with CMake"

cmake -B$build_dir -H. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=g++ \
  -DCMAKE_CXX_STANDARD=20

cd "$build_dir"

echo "Building with 6 parallel jobs"
make -j6