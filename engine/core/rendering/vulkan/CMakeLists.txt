include(VulkanDependencies.cmake)

target_compile_definitions(CorvoriumEngine PUBLIC VULKAN_HPP_NO_EXCEPTIONS VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)
target_link_libraries(CorvoriumEngine
        PRIVATE
        ${Corvorium_VULKAN_LIBS}
)

target_sources(CorvoriumEngine
        PRIVATE
        Vulkan.cpp
        PRIVATE FILE_SET rendering_impl TYPE CXX_MODULES FILES
        VulkanContext.cppm
        VulkanDevice.cppm
        VulkanPipeline.cppm
        VulkanPipelineUtils.cppm
        VulkanSwapchain.cppm
        VulkanCommandBuffer.cppm
        VulkanUtils.cppm

        private/VulkanDevice.cppm
        private/VulkanSwapchain.cppm
)
