

target_sources(CorvoriumEngine
        PUBLIC FILE_SET event_interface TYPE CXX_MODULES FILES
        Event.cppm
        GlobalEventBus.cppm
        EventType.cppm
        #PRIVATE FILE_SET event_impl TYPE CXX_MODULES FILES
)

target_include_directories(CorvoriumEngine PUBLIC ./include)
