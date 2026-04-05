#!/bin/bash

BUILD_DIR_LINUX="./build_linux"
BUILD_DIR_WIN="./build_win"
BUILD_FOR_WINDOWS=0
BUILD_ALL=0
CLEAN_BUILD=0
BUILD_TESTS=0
ENABLE_COVERAGE=0

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
    --test|-t)
      BUILD_TESTS=1
      ;;
    --coverage|-cov)
      BUILD_TESTS=1
      ENABLE_COVERAGE=1
      ;;
    --help)
      echo "Usage: build.sh [--clean|-c] [--windows|-w] [--all|-a] [--test|-t] [--coverage|-cov] [--help]"
      echo ""
      echo "Options:"
      echo "  --clean, -c       Clean build directories before building"
      echo "  --windows, -w     Build for Windows (cross-compile)"
      echo "  --all, -a         Build for both Windows and Linux in parallel"
      echo "  --test, -t        Build and run unit tests"
      echo "  --coverage, -cov Build with coverage instrumentation and generate report"
      exit 0
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
  shift
done

format_code() {
  echo "Formatting source files..."
  find src/ include/models include/panels include/system \
    \( -name "*.cpp" -o -name "*.h" \) \
    | xargs clang-format -i
}

# Handle clean build
if [[ $CLEAN_BUILD -eq 1 ]]; then
  echo "Deleting Build Directories"
  rm -rf "$BUILD_DIR_LINUX" "$BUILD_DIR_WIN"
fi

format_code

if [[ $BUILD_ALL -eq 1 ]]; then
  # Parallel build: both targets run concurrently, each logs to ./logs/
  LOG_DIR="./logs"
  mkdir -p "$LOG_DIR"

  echo "Starting Windows and Linux builds in parallel..."

  (
    cmake -B"$BUILD_DIR_WIN" -H. \
      --log-level=VERBOSE \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_SYSTEM_NAME=Windows \
      -DCMAKE_C_COMPILER="x86_64-w64-mingw32-gcc-posix" \
      -DCMAKE_CXX_COMPILER="x86_64-w64-mingw32-g++-posix"
    make -C"$BUILD_DIR_WIN" -j6 VERBOSE=1
  ) > "$LOG_DIR/build_win.log" 2>&1 &
  WIN_PID=$!

  (
    cmake -B"$BUILD_DIR_LINUX" -H. --log-level=VERBOSE -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20
    make -C"$BUILD_DIR_LINUX" -j6 VERBOSE=1
  ) > "$LOG_DIR/build_linux.log" 2>&1 &
  LIN_PID=$!

  wait $WIN_PID; WIN_EXIT=$?
  wait $LIN_PID; LIN_EXIT=$?

  [[ $WIN_EXIT -eq 0 ]] && echo "Windows: PASS" || echo "Windows: FAIL (see logs/build_win.log)"
  [[ $LIN_EXIT -eq 0 ]] && echo "Linux:   PASS" || echo "Linux:   FAIL (see logs/build_linux.log)"

  [[ $WIN_EXIT -eq 0 && $LIN_EXIT -eq 0 ]]
else
  # Single-target builds: synchronous, set -e active
  set -e

  # Build for Windows
  if [[ $BUILD_FOR_WINDOWS -eq 1 ]]; then
    echo "Configuring Windows build..."
    CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc-posix -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++-posix"
    
    if [[ $BUILD_TESTS -eq 1 ]]; then
      CMAKE_FLAGS="$CMAKE_FLAGS -DBUILD_TESTS=ON"
    fi
    
    if [[ $ENABLE_COVERAGE -eq 1 ]]; then
      CMAKE_FLAGS="$CMAKE_FLAGS -DENABLE_COVERAGE=ON"
    fi
    
    cmake -B"$BUILD_DIR_WIN" -H. --log-level=VERBOSE $CMAKE_FLAGS
    make -C"$BUILD_DIR_WIN" -j6 VERBOSE=1
  else
    # Default: Linux only
    echo "Configuring Linux build..."
    CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20"
    
    if [[ $BUILD_TESTS -eq 1 ]]; then
      CMAKE_FLAGS="$CMAKE_FLAGS -DBUILD_TESTS=ON"
    fi
    
    if [[ $ENABLE_COVERAGE -eq 1 ]]; then
      CMAKE_FLAGS="$CMAKE_FLAGS -DENABLE_COVERAGE=ON"
    fi
    
    cmake -B"$BUILD_DIR_LINUX" -H. --log-level=VERBOSE $CMAKE_FLAGS
    make -C"$BUILD_DIR_LINUX" -j6 VERBOSE=1
    
    # Run tests if requested
    if [[ $BUILD_TESTS -eq 1 ]]; then
      echo "Running tests..."
      if [[ $ENABLE_COVERAGE -eq 1 ]]; then
        # Run tests with coverage
        ./build_linux/WorkbenchTests --gtest_color=yes
        COVERAGE_DIR="./coverage"
        mkdir -p "$COVERAGE_DIR"
        
        if command -v llvm-profdata &> /dev/null && command -v llvm-cov &> /dev/null; then
          echo "Generating coverage report..."
          
          # Collect all profraw files
          PROFRAW_FILES=$(find ./build_linux -name "*.profraw" 2>/dev/null)
          
          if [[ -n "$PROFRAW_FILES" ]]; then
            # Merge profraw files into profdata
            echo "$PROFRAW_FILES" | xargs llvm-profdata merge -o coverage.profdata
            
            # Generate HTML report
            llvm-cov report -instr-profile=coverage.profdata \
              -object=build_linux/tests/WorkbenchTests \
              -sources=src/ \
              -format=html \
              -output-dir="$COVERAGE_DIR"
            
            echo "Coverage report generated at: $COVERAGE_DIR/index.html"
          else
            echo "Warning: No .profraw files found"
          fi
        else
          echo "Warning: llvm-profdata or llvm-cov not found, skipping coverage report"
        fi
      else
        ./build_linux/WorkbenchTests --gtest_color=yes
      fi
    fi
  fi
fi
