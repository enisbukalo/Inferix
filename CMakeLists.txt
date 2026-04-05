cmake_minimum_required(VERSION 3.25)
project(Workbench)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build unit tests" OFF)
option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)

# FTXUI
include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)

# spdlog
FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

# Google Test (only if BUILD_TESTS is ON)
if(BUILD_TESTS)
  FetchContent_Declare(gtest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG v1.14.0
  )
  FetchContent_MakeAvailable(gtest)
endif()

# libvterm third-party library
add_subdirectory(third_party/libvterm-0.3.3)

# Add all subdirectories under include/ as include paths (recursive)
# This ensures includes like "serverSettings.h" work from anywhere
file(GLOB_RECURSE ALL_ITEMS LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/include/*)
set(INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include")
foreach(ITEM ${ALL_ITEMS})
  if(IS_DIRECTORY "${ITEM}")
    list(APPEND INCLUDE_DIRS "${ITEM}")
  endif()
endforeach()
message(STATUS "Include directories: ${INCLUDE_DIRS}")

# httplib header-only interface library
add_library(httplib INTERFACE)
target_include_directories(httplib INTERFACE ${CMAKE_SOURCE_DIR}/include ${INCLUDE_DIRS})

# Collect all sources using glob
file(GLOB_RECURSE ALL_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# Remove platform-specific files
list(REMOVE_ITEM ALL_SOURCES
    "${CMAKE_SOURCE_DIR}/src/system/ramLinux.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/ramWindows.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/cpuLinux.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/cpuWindows.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/systemInfoLinux.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/systemInfoWindows.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/ptyLinux.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/ptyWindows.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/llamaServerProcess.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/llamaServerProcessLinux.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/llamaServerProcessWindows.cpp"
)

# Add back only the files for the current platform
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  list(APPEND ALL_SOURCES
      "${CMAKE_SOURCE_DIR}/src/system/ramWindows.cpp"
      "${CMAKE_SOURCE_DIR}/src/system/cpuWindows.cpp"
      "${CMAKE_SOURCE_DIR}/src/system/systemInfoWindows.cpp"
      "${CMAKE_SOURCE_DIR}/src/system/ptyWindows.cpp"
      "${CMAKE_SOURCE_DIR}/src/system/llamaServerProcess.cpp"
      "${CMAKE_SOURCE_DIR}/src/system/llamaServerProcessWindows.cpp"
  )
else()
  list(APPEND ALL_SOURCES
      "${CMAKE_SOURCE_DIR}/src/system/ramLinux.cpp"
      "${CMAKE_SOURCE_DIR}/src/system/cpuLinux.cpp"
      "${CMAKE_SOURCE_DIR}/src/system/systemInfoLinux.cpp"
      "${CMAKE_SOURCE_DIR}/src/system/ptyLinux.cpp"
      "${CMAKE_SOURCE_DIR}/src/system/llamaServerProcess.cpp"
      "${CMAKE_SOURCE_DIR}/src/system/llamaServerProcessLinux.cpp"
  )
endif()

add_executable(Workbench ${ALL_SOURCES})

# Target settings
target_link_libraries(Workbench PRIVATE httplib ftxui::component ftxui::dom ftxui::screen vterm spdlog::spdlog)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_libraries(Workbench PRIVATE ws2_32)
  target_link_options(Workbench PRIVATE -static-libgcc -static-libstdc++ -static)
else()
  target_link_libraries(Workbench PRIVATE util)
endif()
target_compile_options(Workbench PRIVATE -Wall -Wextra -g -O3)
set_target_properties(Workbench PROPERTIES OUTPUT_NAME "Workbench")

# Coverage instrumentation (Clang)
if(ENABLE_COVERAGE)
  message(STATUS "Coverage instrumentation enabled")
  add_compile_options(-fprofile-instrs-gen -fcoverage-coverage -fcoverage-mapping)
  add_link_options(-fprofile-instrs-gen -fcoverage)
endif()

# Tests subdirectory
if(BUILD_TESTS)
  add_subdirectory(tests)
endif()
