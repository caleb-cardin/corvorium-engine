
find_package(Vulkan REQUIRED COMPONENTS volk)

if (Vulkan_FOUND)

    find_package(Vulkan REQUIRED)

    add_library(VulkanCppModule)
    add_library(Vulkan::cppm ALIAS VulkanCppModule)

    target_sources(VulkanCppModule PUBLIC
            FILE_SET cxx_modules TYPE CXX_MODULES
            BASE_DIRS "${Vulkan_INCLUDE_DIR}"
            FILES "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
    )
    target_link_libraries(VulkanCppModule PUBLIC Vulkan::Vulkan)
    target_compile_definitions(VulkanCppModule PRIVATE
            VULKAN_HPP_NO_SETTERS
            VULKAN_HPP_NO_SMART_HANDLE
            VULKAN_HPP_NO_EXCEPTIONS
            VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

    set(Corvorium_VULKAN_LIBS VulkanMemoryAllocator Vulkan::cppm)
else ()
    message(FATAL_ERROR "CorvoriumRendering | Vulkan sdk library not found!")
    set(Corvorium_VULKAN_LIBS)
endif ()
