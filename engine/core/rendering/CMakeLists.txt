
target_include_directories(CorvoriumEngine PRIVATE ./shader/include)

add_subdirectory(common)
add_subdirectory(shaderCompiler)
add_subdirectory(vulkan)

target_link_libraries(CorvoriumEngine
        PRIVATE
        glfw
        glm
)

target_sources(CorvoriumEngine
        PUBLIC FILE_SET rendering_interface TYPE CXX_MODULES FILES
        Rendering.cppm
        IRenderCore.cppm
        RenderTypes.cppm
        CommandBuffer.cppm
        PRIVATE FILE_SET rendering_impl TYPE CXX_MODULES FILES
        private/IRenderCore.cppm
        private/CommandBuffer.cppm
)

