cmake_minimum_required(VERSION 4.0.1)
project(EventParser VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_subdirectory("../lwjson" lwjson)

# Create EventParser executable (native macOS)
add_executable(EventParser TahomaEventParserTest.cpp)
target_link_libraries(EventParser PUBLIC lwjson)

target_include_directories(EventParser PRIVATE
    ../App
)

target_sources(EventParser PRIVATE
    ../App/TahomaEventParser.cpp
)

set_target_properties(EventParser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)