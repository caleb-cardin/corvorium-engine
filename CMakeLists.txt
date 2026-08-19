cmake_minimum_required(VERSION 4.1)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

project(CorvoriumEngine CXX)

add_subdirectory(lib SYSTEM)
add_subdirectory(engine)
