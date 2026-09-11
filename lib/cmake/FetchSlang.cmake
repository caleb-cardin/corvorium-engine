include_guard()

include(FetchContent)

if (CORVORIUM_PLATFORM_WINDOWS)
    set(slang_URL "https://github.com/shader-slang/slang/releases/download/v2026.17/slang-2026.17-windows-x86_64.zip")
elseif(CORVORIUM_PLATFORM_LINUX)
    set(slang_URL "https://github.com/shader-slang/slang/releases/download/v2026.17/slang-2026.17-linux-x86_64.zip")
else ()
    message(FATAL_ERROR "Unsupported platform")
endif ()

FetchContent_Declare(slang_release URL ${slang_URL})
FetchContent_MakeAvailable(slang_release)

set(slang_DIR "${slang_release_SOURCE_DIR}/cmake")
find_package(slang CONFIG REQUIRED)
set_target_properties(slang::slang PROPERTIES IMPORTED_GLOBAL TRUE)

