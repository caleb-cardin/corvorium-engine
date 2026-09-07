include(VulkanDependencies.cmake)

target_compile_definitions(CorvoriumEngine PRIVATE
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)
target_link_libraries(CorvoriumEngine
        PRIVATE
        ${Corvorium_VULKAN_LIBS}
)

target_sources(CorvoriumEngine
        PRIVATE
        Vulkan.cpp
        PRIVATE FILE_SET vulkan_impl TYPE CXX_MODULES FILES
        VulkanCore.cppm
        VulkanImage.cppm
        VulkanPipeline.cppm
        VulkanPipelineUtils.cppm
        VulkanSwapchain.cppm
        VulkanShaderCache.cppm
        VulkanCommandBuffer.cppm
        VulkanUtils.cppm

        private/VulkanCore.cppm
        private/VulkanImage.cppm
        private/VulkanPipeline.cppm
        private/VulkanSwapchain.cppm
)
