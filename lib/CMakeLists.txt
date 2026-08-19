

add_subdirectory(EASTL)
target_compile_options(EASTL PRIVATE /wd5311)
add_subdirectory(glfw)
add_subdirectory(mimalloc)

set(slang_DIR "${CMAKE_CURRENT_SOURCE_DIR}/slang/cmake")
find_package(slang CONFIG REQUIRED)
set_target_properties(slang::slang PROPERTIES IMPORTED_GLOBAL TRUE)

add_subdirectory(spdlog)
add_subdirectory(vma)
