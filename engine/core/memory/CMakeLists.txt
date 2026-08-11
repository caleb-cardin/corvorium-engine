target_sources(CorvoriumEngine
        PUBLIC FILE_SET memory_interface TYPE CXX_MODULES FILES
        Memory.cppm
        PRIVATE FILE_SET memory_impl TYPE CXX_MODULES FILES
        Memory_impl.cppm
)
