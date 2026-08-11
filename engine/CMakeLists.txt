
add_library(CorvoriumEngine STATIC)

if(MSVC)
    target_compile_options(CorvoriumEngine PRIVATE /std:c++latest /EHsc-)
else()
    # For Clang/GCC presets
    target_compile_options(CorvoriumEngine PRIVATE -std=c++2c)
endif()

add_subdirectory(core)
