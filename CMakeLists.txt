cmake_minimum_required(VERSION 3.16)
project(DevEscape VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(SDL2 CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

# Core framework sources
set(FRAMEWORK_SOURCES
    src/main.cpp
    src/framework/PluginManager.cpp
    src/framework/AudioManager.cpp
    src/framework/StateManager.cpp
    src/framework/TerminalRenderer.cpp
    src/framework/TerminalControl.cpp
    src/framework/TimerSystem.cpp
    src/framework/HintSystem.cpp
    src/framework/GameLoop.cpp
)

# Main executable
add_executable(devescape ${FRAMEWORK_SOURCES})

target_link_libraries(devescape
    ${SDL2_LIBRARIES}
    ${Boost_LIBRARIES}
    nlohmann_json::nlohmann_json
    ${CMAKE_DL_LIBS}
)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    target_link_libraries(devescape pthread)
endif()

# Install targets
install(TARGETS devescape DESTINATION bin)

# Add plugin subdirectories
add_subdirectory(plugins/production_incident)
