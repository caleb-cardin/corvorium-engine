
add_library(CorvoriumEngine STATIC)

target_link_libraries(CorvoriumEngine PUBLIC EASTL)

target_compile_definitions(CorvoriumEngine
    PRIVATE
        CORVORIUM_VERSION_MAJOR=26
        CORVORIUM_VERSION_MINOR=1
        CORVORIUM_VERSION_PATCH=0
)

if(MSVC)
    target_compile_options(CorvoriumEngine PRIVATE /std:c++latest /EHsc-)
    target_compile_definitions(CorvoriumEngine
        PRIVATE
            CORVORIUM_WINDOWS=1
    )
else()
    # For Clang/GCC presets
    target_compile_options(CorvoriumEngine PRIVATE -std=c++2c)
endif()

target_compile_definitions(CorvoriumEngine
    PRIVATE
        $<$<CONFIG:Debug>:CORVORIUM_DEBUG>
        $<$<CONFIG:Debug>:CORVORIUM_BUILD_MODE=2>
        $<$<CONFIG:RelWithDebugInfo>:CORVORIUM_DEVELOPMENT>
        $<$<CONFIG:RelWithDebugInfo>:CORVORIUM_BUILD_MODE=1>
        $<$<CONFIG:Release>:CORVORIUM_RELEASE>
        $<$<CONFIG:Release>:CORVORIUM_BUILD_MODE=0>
)

add_subdirectory(core)
