# Tests CMakeLists.txt

# Create test executable
add_executable(WorkbenchTests
    main.cpp
    config/config_serialization_test.cpp
    config/config_manager_test.cpp
    config/config_manager_presets_test.cpp
    core/event_bus_test.cpp
    utility/model_discovery_test.cpp
    utility/models_ini_test.cpp
    system/cpu_monitor_test.cpp
    system/memory_monitor_test.cpp
    system/gpu_monitor_test.cpp
    system/llama_server_args_test.cpp
    system/http_client_test.cpp
    system/pty_handler_test.cpp
    system/model_info_monitor_test.cpp
)

# Set output directory to match main build
set_target_properties(WorkbenchTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Link gtest
target_link_libraries(WorkbenchTests PRIVATE
    GTest::gtest_main
    GTest::gtest
)

# Link required libraries from main target
target_link_libraries(WorkbenchTests PRIVATE
    httplib
    spdlog::spdlog
    ftxui::component
    ftxui::dom
    ftxui::screen
)

# Include directories (same as main target)
target_include_directories(WorkbenchTests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${INCLUDE_DIRS}
)

# Platform-specific sources based on config in parent
# Reuse the same source selection logic as main executable
file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_SOURCE_DIR}/src/config/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/utility/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/system/*.cpp"
)

# Remove platform-specific files (same pattern as main)
list(REMOVE_ITEM TEST_SOURCES
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
    "${CMAKE_SOURCE_DIR}/src/system/systemMonitorRunner.cpp"
)

# Add back only the files for the current platform
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND TEST_SOURCES
        "${CMAKE_SOURCE_DIR}/src/system/ramWindows.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/cpuWindows.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/systemInfoWindows.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/ptyWindows.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/llamaServerProcess.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/llamaServerProcessWindows.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/cpuMonitor.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/ramMonitor.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/gpuMonitor.cpp"
    )
else()
    list(APPEND TEST_SOURCES
        "${CMAKE_SOURCE_DIR}/src/system/ramLinux.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/cpuLinux.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/systemInfoLinux.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/ptyLinux.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/llamaServerProcess.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/llamaServerProcessLinux.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/cpuMonitor.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/ramMonitor.cpp"
        "${CMAKE_SOURCE_DIR}/src/system/gpuMonitor.cpp"
    )
endif()

target_sources(WorkbenchTests PRIVATE ${TEST_SOURCES})

# Coverage settings
if(ENABLE_COVERAGE)
    message(STATUS "Coverage instrumentation enabled for tests")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(WorkbenchTests PRIVATE -fprofile-instrs-gen -fcoverage-coverage -fcoverage-mapping)
        target_link_options(WorkbenchTests PRIVATE -fprofile-instrs-gen -fcoverage)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(WorkbenchTests PRIVATE -fprofile-arcs -ftest-coverage)
        target_link_options(WorkbenchTests PRIVATE -fprofile-arcs -ftest-coverage)
    endif()
endif()

# Enable testing
enable_testing()
add_test(NAME WorkbenchTests COMMAND WorkbenchTests)