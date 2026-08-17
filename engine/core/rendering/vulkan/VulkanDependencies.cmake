
find_package(Vulkan REQUIRED COMPONENTS volk)

if (Vulkan_FOUND)
    message(STATUS "CorvoriumRendering | Vulkan sdk library found : version = ${Vulkan_VERSION}")

    find_package (Vulkan REQUIRED)

    # set up Vulkan C++ module (enabled when ENABLE_CPP20_MODULE=ON)
    add_library(VulkanCppModule)
    add_library(Vulkan::cppm ALIAS VulkanCppModule)

    target_compile_definitions(VulkanCppModule PUBLIC
            VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
            VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
            VULKAN_HPP_CXX_MODULE_EXPERIMENTAL_WARNING
    )

    target_include_directories(VulkanCppModule PUBLIC "${Vulkan_INCLUDE_DIR}")

    target_link_libraries(VulkanCppModule PUBLIC Vulkan::Vulkan)

    set_target_properties(VulkanCppModule PROPERTIES CXX_STANDARD 20)

    target_sources(VulkanCppModule
            PUBLIC
            FILE_SET cxx_modules TYPE CXX_MODULES
            BASE_DIRS "${Vulkan_INCLUDE_DIR}"
            FILES "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
    )

    set(Corvorium_VULKAN_LIBS Vulkan::Vulkan VulkanMemoryAllocator)
else()
    message(FATAL_ERROR "CorvoriumRendering | Vulkan sdk library not found!")
    set(Corvorium_VULKAN_LIBS)
endif ()
