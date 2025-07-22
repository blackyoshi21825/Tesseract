cmake_minimum_required(VERSION 3.10)
project(Tesseract C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable precompiled headers if supported
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
    set(CMAKE_UNITY_BUILD ON)
    set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)
    set(CMAKE_UNITY_BUILD_BATCH_SIZE 8)
endif()

# Set build type to Release by default for better performance
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add include directory
include_directories(include)

# Set compiler flags
if(MSVC)
    add_compile_options(/W4 /O2 /GL /MP)
    add_link_options(/LTCG)
else()
    add_compile_options(-Wall -Wextra -std=c99 -O3 -ffast-math -flto -march=native)
    add_link_options(-flto)
endif()

# Find source files
file(GLOB SOURCES "src/*.c")

# Find curl package
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

# Create executable
add_executable(tesser ${SOURCES})

# Add precompiled header if supported
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
    target_precompile_headers(tesser PRIVATE include/tesseract_pch.h)
endif()

# Link math and curl libraries
target_link_libraries(tesser m ${CURL_LIBRARIES})

# Add custom targets
add_custom_target(debug
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --config Debug
    COMMENT "Building with debug flags"
)

add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/Testing
    COMMENT "Cleaning all build files"
)