
target_sources(CorvoriumEngine
        PUBLIC FILE_SET rendering_common_interface TYPE CXX_MODULES FILES
        RenderingCommon.cppm
        #PRIVATE FILE_SET rendering_common_impl TYPE CXX_MODULES FILES
)
