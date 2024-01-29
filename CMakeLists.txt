cmake_minimum_required(VERSION 3.27)
project(GGJ24)

set(CMAKE_CXX_STANDARD 20)

# Find raylib
find_package(raylib REQUIRED)

add_executable(GGJ24 main.cpp)

#link agaisnt raylib library
target_link_libraries(GGJ24 raylib "-framework IOKit" "-framework Cocoa" "-framework OpenGL")

if(APPLE)
    set_target_properties(GGJ24 PROPERTIES
                            MACOSX_BUNDLE TRUE
                            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
                            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
                            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
                            XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO
                            XCODE_ATTRIBUTE_ARCHS "arm64")
endif()