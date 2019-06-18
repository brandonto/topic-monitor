# Minimum required CMake version
cmake_minimum_required(VERSION 3.5.1)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)

# Project name
project(topic-monitor)

# Include directories
include_directories()

# Link directories
link_directories()

# Executable
set(EXECUTABLE_NAME "topic-monitor")
set(SOURCE_FILES main.cpp solClientThread.cpp monitoringThread.cpp utils.cpp)
add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})

# Enable all warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")

# Link lbraries
target_link_libraries(${PROJECT_NAME} solclient lua5.2)

