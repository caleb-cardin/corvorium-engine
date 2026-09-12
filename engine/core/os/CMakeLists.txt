
target_compile_definitions(CorvoriumEngine PUBLIC VULKAN_HPP_NO_EXCEPTIONS)
target_link_libraries(CorvoriumEngine
        PRIVATE
        EASTL
        glfw
)


target_sources(CorvoriumEngine
        PUBLIC FILE_SET os_interface TYPE CXX_MODULES FILES
        OS.cppm
        FileProvider.cppm
        window/Window.cppm
        window/WindowManager.cppm
        window/WindowCallbackData.cppm

        input/Input.cppm
        input/InputManager.cppm
        input/InputTypes.cppm

        PRIVATE FILE_SET os_impl TYPE CXX_MODULES FILES
        window/private/Window.cppm
        window/private/WindowManager.cppm
        window/private/WindowCallbackData.cppm

        input/private/InputConversionHelpers.cppm
        input/private/InputManager.cppm
)
