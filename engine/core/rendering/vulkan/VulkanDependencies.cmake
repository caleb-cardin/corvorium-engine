
find_package(Vulkan REQUIRED COMPONENTS volk)

if (Vulkan_FOUND)

    find_package(Vulkan REQUIRED)
    
    set(Corvorium_VULKAN_LIBS Vulkan::Vulkan VulkanMemoryAllocator)
else ()
    message(FATAL_ERROR "CorvoriumRendering | Vulkan sdk library not found!")
    set(Corvorium_VULKAN_LIBS)
endif ()
