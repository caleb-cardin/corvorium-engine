

set(_saved_message_log_level ${CMAKE_MESSAGE_LOG_LEVEL})
set(CMAKE_MESSAGE_LOG_LEVEL WARNING)

add_subdirectory(EASTL)
target_compile_options(EASTL PRIVATE /wd5311)

add_subdirectory(glfw)
add_subdirectory(glm)
target_compile_definitions(glm
        PRIVATE
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_LEFT_HANDED
        GLM_FORCE_INLINE
)

set(MI_BUILD_TESTS OFF)
add_subdirectory(mimalloc)

set(slang_DIR "${CMAKE_CURRENT_SOURCE_DIR}/slang/cmake")
find_package(slang CONFIG REQUIRED)
set_target_properties(slang::slang PROPERTIES IMPORTED_GLOBAL TRUE)

add_subdirectory(spdlog)
add_subdirectory(vma)

set(CMAKE_MESSAGE_LOG_LEVEL _saved_message_log_level)
