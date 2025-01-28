cmake_minimum_required(VERSION 3.14)

# Set the project name
project(LamportAlgorithm CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# Add the executable
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(LamportAlgorithm main.cpp ${SOURCES})

# Include directories
include_directories(include)

find_package(OpenSSL REQUIRED)

# Link necessary libraries for network and threading functionality
target_link_libraries(LamportAlgorithm ${CMAKE_THREAD_LIBS} ${CMAKE_DL_LIBS} OpenSSL::Crypto)

# Custom build target for creating only the main executable
add_custom_target(build
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target LamportAlgorithm
    DEPENDS LamportAlgorithm
)

# Custom clean target to remove all build files
add_custom_target(cleanup
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/*
    COMMENT "Cleaning all build files..."
)