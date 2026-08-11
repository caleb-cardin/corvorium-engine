
include(vulkan/VulkanDependencies.cmake)

target_link_libraries(CorvoriumEngine
    PRIVATE
        ${Corvorium_VULKAN_LIBS}
        slang
)

target_sources(CorvoriumEngine
        PUBLIC FILE_SET rendering_interface TYPE CXX_MODULES FILES
        Rendering.cppm
        RenderInstance.cppm
        PRIVATE FILE_SET rendering_impl TYPE CXX_MODULES FILES
)
