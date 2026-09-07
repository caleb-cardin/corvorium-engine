cmake_minimum_required(VERSION 4.1)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)


set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")

project(CorvoriumEngine CXX)

add_subdirectory(lib SYSTEM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_MODULE_STD ON)

add_subdirectory(engine)

set(CMAKE_CORVORIUM_RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")

add_subdirectory(test/testApp)
