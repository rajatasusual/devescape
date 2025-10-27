cmake_minimum_required(VERSION 3.16)
project(DevEscape VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(SDL2 CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(nlohmann_json CONFIG REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# Framework sources (to be built as a shared library)
set(FRAMEWORK_SOURCES
    src/framework/PluginManager.cpp
    src/framework/AudioManager.cpp
    src/framework/StateManager.cpp
    src/framework/TerminalRenderer.cpp
    src/framework/TerminalControl.cpp
    src/framework/DataTypes.cpp
    src/framework/TimerSystem.cpp
    src/framework/HintSystem.cpp
    src/framework/GameLoop.cpp
)

# Create SHARED library for the framework
add_library(devescape_framework SHARED ${FRAMEWORK_SOURCES})

target_link_libraries(devescape_framework PUBLIC
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    Boost::filesystem
    nlohmann_json::nlohmann_json
)

# Export symbols on Windows
if(WIN32)
    target_compile_definitions(devescape_framework PRIVATE DEVESCAPE_EXPORTS)
    target_compile_definitions(devescape_framework INTERFACE DEVESCAPE_IMPORTS)
endif()

# Main executable (just contains main.cpp)
add_executable(devescape src/main.cpp)

target_link_libraries(devescape PRIVATE
    devescape_framework
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
)

# Platform-specific settings
if(WIN32)
    # Windows: Make sure framework DLL is in same directory as exe
    set_target_properties(devescape_framework PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(devescape_framework PUBLIC pthread dl)
elseif(APPLE)
    target_link_libraries(devescape_framework PUBLIC pthread dl)
endif()

# Install targets
install(TARGETS devescape devescape_framework DESTINATION bin)

# Add plugin subdirectories
add_subdirectory(plugins/production_incident)
