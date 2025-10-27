cmake_minimum_required(VERSION 3.16)

project(production_incident_plugin)

set(CMAKE_CXX_STANDARD 17)

# Plugin sources
set(PLUGIN_SOURCES
    ProductionIncidentRoom.cpp
    puzzles/AlertAnalysis.cpp
    puzzles/MetricsNavigation.cpp
    puzzles/PoolOptimization.cpp
    puzzles/ConfigDeployment.cpp
)

# Create shared library
add_library(production_incident SHARED ${PLUGIN_SOURCES})

target_include_directories(production_incident PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Link against the framework library (this resolves the symbols)
target_link_libraries(production_incident PRIVATE
    devescape_framework
    nlohmann_json::nlohmann_json
)

# Set output directory
set_target_properties(production_incident PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

# On Windows, copy framework DLL to plugin directory
if(WIN32)
    add_custom_command(TARGET production_incident POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:devescape_framework>
        ${CMAKE_BINARY_DIR}/plugins/$<CONFIG>/
    )
endif()

install(TARGETS production_incident DESTINATION plugins)
