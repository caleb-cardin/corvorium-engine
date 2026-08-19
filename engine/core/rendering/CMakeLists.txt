
include(vulkan/VulkanDependencies.cmake)

target_compile_definitions(CorvoriumEngine PUBLIC VULKAN_HPP_NO_EXCEPTIONS)
target_link_libraries(CorvoriumEngine
        PRIVATE
        ${Corvorium_VULKAN_LIBS}
        slang::slang
        glfw
)


target_sources(CorvoriumEngine
        PUBLIC FILE_SET rendering_interface TYPE CXX_MODULES FILES
        Rendering.cppm
        RenderContext.cppm
        RenderDevice.cppm
        PRIVATE FILE_SET rendering_impl TYPE CXX_MODULES FILES
        vulkan/VulkanContext.cppm
        vulkan/VulkanDevice.cppm
        vulkan/VulkanPipeline.cppm
        vulkan/VulkanSwapchain.cppm
        vulkan/VulkanUtils.cppm
)
