target_sources(CorvoriumEngine
        PUBLIC FILE_SET application_interface TYPE CXX_MODULES FILES
        Application.cppm
        #PRIVATE FILE_SET application_impl TYPE CXX_MODULES FILES

)
