cmake_minimum_required(VERSION 3.25)
project(Inferix)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# FTXUI
include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)

# Collect all subdirectories under include/ as include paths
file(GLOB INCLUDE_SUBDIRS LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/include/*)

# httplib header-only interface library
add_library(httplib INTERFACE)
target_include_directories(httplib INTERFACE ${INCLUDE_SUBDIRS})

# Collect all sources using glob
file(GLOB_RECURSE ALL_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# Remove platform-specific files
list(REMOVE_ITEM ALL_SOURCES
    "${CMAKE_SOURCE_DIR}/src/system/ram_linux.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/ram_windows.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/cpu_linux.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/cpu_windows.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/system_info_linux.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/system_info_windows.cpp"
)

# Add back only the files for the current platform
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  list(APPEND ALL_SOURCES
      "${CMAKE_SOURCE_DIR}/src/system/ram_windows.cpp"
      "${CMAKE_SOURCE_DIR}/src/system/cpu_windows.cpp"
      "${CMAKE_SOURCE_DIR}/src/system/system_info_windows.cpp"
  )
else()
  list(APPEND ALL_SOURCES
      "${CMAKE_SOURCE_DIR}/src/system/ram_linux.cpp"
      "${CMAKE_SOURCE_DIR}/src/system/cpu_linux.cpp"
      "${CMAKE_SOURCE_DIR}/src/system/system_info_linux.cpp"
  )
endif()

add_executable(Inferix ${ALL_SOURCES})

# Target settings
target_link_libraries(Inferix PRIVATE httplib ftxui::component ftxui::dom ftxui::screen)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_libraries(Inferix PRIVATE ws2_32)
  target_link_options(Inferix PRIVATE -static-libgcc -static-libstdc++ -static)
endif()
target_compile_options(Inferix PRIVATE -Wall -Wextra -g -O3)
set_target_properties(Inferix PROPERTIES OUTPUT_NAME "Inferix")
