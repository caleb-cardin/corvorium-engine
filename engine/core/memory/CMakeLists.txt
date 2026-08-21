target_sources(CorvoriumEngine
        PRIVATE
        New.cpp
        #PUBLIC FILE_SET memory_interface TYPE CXX_MODULES FILES
        #PRIVATE FILE_SET memory_impl TYPE CXX_MODULES FILES
)


target_link_libraries(CorvoriumEngine PUBLIC mimalloc)
