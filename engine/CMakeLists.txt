
add_library(CorvoriumEngine SHARED)

target_link_libraries(CorvoriumEngine PUBLIC EASTL)

target_link_options(CorvoriumEngine PRIVATE /INCREMENTAL:NO)
target_compile_definitions(CorvoriumEngine
        PRIVATE
        CORVORIUM_VERSION_MAJOR=26
        CORVORIUM_VERSION_MINOR=1
        CORVORIUM_VERSION_PATCH=0
)

# For dll import/export definitions
target_compile_definitions(CorvoriumEngine PRIVATE CORVORIUM_ENGINE_BUILD)
target_include_directories(CorvoriumEngine PRIVATE ./core/include)

if (MSVC)
    target_compile_options(CorvoriumEngine PRIVATE /std:c++latest /EHsc-)
    target_compile_definitions(CorvoriumEngine
            PRIVATE
            CORVORIUM_WINDOWS=1
    )
else ()
    # For Clang/GCC presets
    target_compile_options(CorvoriumEngine PRIVATE -std=c++2c)
endif ()

target_compile_definitions(CorvoriumEngine
        PRIVATE
        $<$<CONFIG:Debug>:CORVORIUM_DEBUG>
        $<$<CONFIG:Debug>:CORVORIUM_BUILD_MODE=2>
        $<$<CONFIG:RelWithDebInfo>:CORVORIUM_DEVELOPMENT>
        $<$<CONFIG:RelWithDebInfo>:CORVORIUM_BUILD_MODE=1>
        $<$<CONFIG:Release>:CORVORIUM_RELEASE>
        $<$<CONFIG:Release>:CORVORIUM_BUILD_MODE=0>
)

add_subdirectory(core)
