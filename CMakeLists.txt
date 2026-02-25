cmake_minimum_required(VERSION 3.16)
project(Inferix)

# Find and include httplib
# This assumes httplib is in the project directory or in a standard path
# If it's a dependency, you'd use find_package or add_subdirectory

# Add the httplib header file to the build
add_library(httplib INTERFACE)

# Specify header path (assuming httplib.h is in include/httplib directory)
set(HTTPLIB_HEADER_PATH "${CMAKE_SOURCE_DIR}/include/httplib")

# Precompile headers - this is the key line they mentioned
# Note: The documentation uses target_precompile_headers with the target name
# It needs to be BEFORE target_link_libraries or target_link_directories
# In this case, it's already being added to the INTERFACE target

# If httplib is built separately, you'd use:
# target_precompile_headers(httplib::httplib INTERFACE ${HTTPLIB_HEADER_PATH})

# Add executable
add_executable(Inferix main.cpp)

# Link dependencies
# If httplib needs linking (e.g., with SSL), uncomment the following:
# target_link_libraries(Inferix PRIVATE httplib::httplib)

# Set compiler flags for C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall -Wextra -g -O3")