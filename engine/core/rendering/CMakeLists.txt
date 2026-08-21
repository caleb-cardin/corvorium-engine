
include(vulkan/VulkanDependencies.cmake)

target_compile_definitions(CorvoriumEngine PUBLIC VULKAN_HPP_NO_EXCEPTIONS VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)
target_link_libraries(CorvoriumEngine
        PRIVATE
        ${Corvorium_VULKAN_LIBS}
        slang::slang
        glfw
)


target_sources(CorvoriumEngine
        PRIVATE
        vulkan/Vulkan.cpp
        PUBLIC FILE_SET rendering_interface TYPE CXX_MODULES FILES
        Rendering.cppm
        RenderContext.cppm
        RenderDevice.cppm
        RenderTypes.cppm
        Shader.cppm
        PRIVATE FILE_SET rendering_impl TYPE CXX_MODULES FILES
        private/ShaderCompiler.cppm
        private/RenderContext.cppm
        vulkan/VulkanContext.cppm
        vulkan/VulkanDevice.cppm
        vulkan/VulkanPipeline.cppm
        vulkan/VulkanSwapchain.cppm
        vulkan/VulkanSwapchainImpl.cppm
        vulkan/VulkanUtils.cppm
)
