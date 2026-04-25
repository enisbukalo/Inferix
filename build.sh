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
      ENABLE_COVERAGE=1
      ;;
    --coverage|-cov)
      BUILD_TESTS=1
      ENABLE_COVERAGE=1
      ;;
    --help)
      echo "Usage: build.sh [--clean|-c] [--windows|-w] [--all|-a] [--test|-t] [--help]"
      echo ""
      echo "Options:"
      echo "  --clean, -c       Clean build directories before building"
      echo "  --windows, -w     Build for Windows (cross-compile)"
      echo "  --all, -a         Build for both Windows and Linux in parallel"
      echo "  --test, -t        Build and run unit tests with coverage report"
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
      LOG_DIR="./logs"
      mkdir -p "$LOG_DIR"
      TEST_LOG="$LOG_DIR/test_output.log"

      # Run tests (continue even if tests fail, to generate coverage)
      ./build_linux/WorkbenchTests --gtest_color=yes 2>&1 | tee "$TEST_LOG" || true
      
      # Generate coverage report using gcov (GCC format)
      if command -v gcov &> /dev/null; then
        echo ""
        echo "========================================"
        echo "         CODE COVERAGE REPORT"
        echo "========================================"
        
        cd ./build_linux

        # Run gcov on all source gcno files, filter to project source files only
        gcov -b $(find . -path "./tests/CMakeFiles/WorkbenchTests.dir/__/src/*.gcno" 2>/dev/null) 2>/dev/null > /tmp/gcov_output.txt 2>&1

        # Parse gcov output, only counting files under our src/ directory
        awk '
          /^File .*\/app\/src\//{active=1; next}
          /^File /{active=0; next}
          active && /^Lines executed:/{
            split($0, a, ":")
            split(a[2], b, "% of ")
            pct = b[1] + 0
            lines = b[2] + 0
            if (lines > 0) {
              total += lines
              exec += lines * pct / 100
            }
            active=0
          }
        END {
          if (total > 0)
            printf "Total lines: %d\nExecuted: %d\nOVERALL COVERAGE: %.1f%%\n", total, exec, exec*100/total
          else
            print "No coverage data"
        }' /tmp/gcov_output.txt

        cd ..
      fi
    fi
  fi
fi
