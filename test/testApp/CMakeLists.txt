
add_executable(CorvoriumTestApp)

target_sources(CorvoriumTestApp PRIVATE TestApp.cpp)

target_link_libraries(CorvoriumTestApp PRIVATE CorvoriumEngine)

set(RUNTIME_DEPS
        CorvoriumEngine
        mimalloc
)

add_custom_command(TARGET CorvoriumTestApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE_DIR:mimalloc>/mimalloc-redirect.dll
        $<TARGET_FILE_DIR:CorvoriumTestApp>
)

add_custom_command(TARGET CorvoriumTestApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_PDB_FILE:CorvoriumEngine>
        $<TARGET_FILE_DIR:CorvoriumTestApp>
)

foreach (dep ${RUNTIME_DEPS})
    add_custom_command(TARGET CorvoriumTestApp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${dep}>
            $<TARGET_FILE_DIR:CorvoriumTestApp>
    )
endforeach ()
