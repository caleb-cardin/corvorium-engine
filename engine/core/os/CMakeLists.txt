
target_compile_definitions(CorvoriumEngine PUBLIC VULKAN_HPP_NO_EXCEPTIONS)
target_link_libraries(CorvoriumEngine
        PRIVATE
        EASTL
        glfw
)


target_sources(CorvoriumEngine
        PUBLIC FILE_SET os_interface TYPE CXX_MODULES FILES
        OS.cppm
        Window.cppm
        #PRIVATE FILE_SET os_impl TYPE CXX_MODULES FILES

)
