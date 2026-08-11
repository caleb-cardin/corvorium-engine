
find_package(Vulkan REQUIRED COMPONENTS volk)
include(FetchContent)

if (Vulkan_FOUND)
    message(STATUS "CorvoriumRendering | Vulkan sdk library found : version = ${Vulkan_VERSION}")


    # Vulkan components

    if (Vulkan_volk_FOUND)
        message(STATUS "CorvoriumRendering | Vulkan:volk library found")
    else ()
        message(FATAL_ERROR "CorvoriumRendering | Vulkan:volk library not found!")
    endif ()

    set(Corvorium_VULKAN_LIBS Vulkan::Vulkan Vulkan::volk VulkanMemoryAllocator)
else()
    message(FATAL_ERROR "CorvoriumRendering | Vulkan sdk library not found!")
    set(Corvorium_VULKAN_LIBS)
endif ()

