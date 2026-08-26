
target_link_libraries(CorvoriumEngine
        PRIVATE
        slang::slang
        glfw
        glm
)

target_sources(CorvoriumEngine
        PUBLIC FILE_SET rendering_interface TYPE CXX_MODULES FILES
        Rendering.cppm
        RenderContext.cppm
        RenderDevice.cppm
        RenderTypes.cppm
        CommandBuffer.cppm
        PRIVATE FILE_SET rendering_impl TYPE CXX_MODULES FILES
        private/ShaderCompiler.cppm
        private/RenderContext.cppm
        private/CommandBuffer.cppm
)

add_subdirectory(vulkan)
