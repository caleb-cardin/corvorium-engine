
set(_saved_message_log_level ${CMAKE_MESSAGE_LOG_LEVEL})
set(CMAKE_MESSAGE_LOG_LEVEL WARNING)

include(cmake/FetchISPC.cmake)
include(cmake/FetchSlang.cmake)

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


add_subdirectory(spdlog)
add_subdirectory(vma)

set(CMAKE_MESSAGE_LOG_LEVEL _saved_message_log_level)


# TEST
add_library(ImGuiTestLib)

target_sources(ImGuiTestLib
    PUBLIC
        ./imgui/backends/imgui_impl_vulkan.h
    PRIVATE
        ./imgui/backends/imgui_impl_vulkan.cpp
)
find_package(Vulkan)
target_link_libraries(ImGuiTestLib PUBLIC Vulkan::Vulkan imgui)
target_include_directories(ImGuiTestLib PUBLIC ./imgui)

