target_sources(CorvoriumEngine
        PUBLIC FILE_SET common_interface TYPE CXX_MODULES FILES
        Build.cppm
        Debug.cppm
        Flags.cppm
        Logging.cppm
        Result.cppm
        TypeDefs.cppm
        Types.cppm

        PRIVATE FILE_SET common_impl TYPE CXX_MODULES FILES
        private/Logging.cppm
)

target_include_directories(CorvoriumEngine PUBLIC ./include PRIVATE ./private)

target_link_libraries(CorvoriumEngine PRIVATE
        spdlog::spdlog)
