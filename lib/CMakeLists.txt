

add_subdirectory(EASTL)

set(slang_DIR "${CMAKE_CURRENT_SOURCE_DIR}/slang/cmake")
find_package(slang)

add_subdirectory(spdlog)
add_subdirectory(vma)
