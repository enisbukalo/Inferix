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

# httplib header-only interface library
add_library(httplib INTERFACE)
target_include_directories(httplib INTERFACE ${CMAKE_SOURCE_DIR}/include)

# Executable
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_executable(Inferix
    src/main.cpp
    src/ram_windows.cpp
  )
  target_link_libraries(Inferix PRIVATE httplib ftxui::component ftxui::dom ftxui::screen ws2_32 wsock32 -static)
  target_compile_options(Inferix PRIVATE -Wall -Wextra -g -O3)
  set_target_properties(Inferix PROPERTIES OUTPUT_NAME "Inferix")
else()
  add_executable(Inferix
    src/main.cpp
    src/ram_linux.cpp
  )
  target_link_libraries(Inferix PRIVATE httplib ftxui::component ftxui::dom ftxui::screen)
  target_compile_options(Inferix PRIVATE -Wall -Wextra -g -O3)
endif()
