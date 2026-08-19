
target_compile_definitions(CorvoriumEngine PUBLIC VULKAN_HPP_NO_EXCEPTIONS)
target_link_libraries(CorvoriumEngine
        PRIVATE
        EASTL
)


target_sources(CorvoriumEngine
        PUBLIC FILE_SET vfs_interface TYPE CXX_MODULES FILES
        VFS.cppm
        VirtualFileSystem.cppm
        Types.cppm
        PRIVATE FILE_SET vfs_impl TYPE CXX_MODULES FILES
        private/BackendInterface.cppm
        private/MountTable.cppm
        private/VirtualFileSystem.cppm
        private/backends/LooseFileSyncBackend.cppm
)

target_include_directories(CorvoriumEngine PUBLIC ./include)
