include_guard()

include(FetchContent)

if (CORVORIUM_PLATFORM_WINDOWS)
    set(ispc_URL "https://github.com/ispc/ispc/releases/download/v1.31.0/ispc-v1.31.0-windows.zip")
else ()
    message(FATAL_ERROR "Unsupported platform")
endif ()

FetchContent_Declare(ispc_bin URL ${ispc_URL})
FetchContent_MakeAvailable(ispc_bin)
set(CMAKE_ISPC_COMPILER ${ispc_bin_SOURCE_DIR}/bin/ispc.exe)
set(CMAKE_ISPC_INSTRUCTION_SETS avx2-i32x8 CACHE INTERNAL "")
