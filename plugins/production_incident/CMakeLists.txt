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

target_link_libraries(production_incident
    nlohmann_json::nlohmann_json
)

# Set output directory
set_target_properties(production_incident PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

install(TARGETS production_incident DESTINATION plugins)
