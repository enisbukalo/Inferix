cmake_minimum_required(VERSION 3.25)
project(Inferix)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# httplib header-only interface library
add_library(httplib INTERFACE)
target_include_directories(httplib INTERFACE ${CMAKE_SOURCE_DIR}/include/httplib)

# Executable
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_executable(Inferix main.cpp)
  target_link_libraries(Inferix PRIVATE httplib ws2_32 wsock32 -static)
  target_compile_options(Inferix PRIVATE -Wall -Wextra -g -O3)
  set_target_properties(Inferix PROPERTIES OUTPUT_NAME "Inferix")
else()
  add_executable(Inferix main.cpp)
  target_link_libraries(Inferix PRIVATE httplib)
  target_compile_options(Inferix PRIVATE -Wall -Wextra -g -O3)
endif()
