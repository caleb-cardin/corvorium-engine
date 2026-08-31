
target_link_libraries(CorvoriumEngine
        PRIVATE
        slang::slang
)


target_sources(CorvoriumEngine
        PUBLIC FILE_SET shader_compiler_interface TYPE CXX_MODULES FILES
        ShaderCompiler.cppm
        Compiler.cppm
        CompileContext.cppm
        ShaderCompilerTypes.cppm
        ShaderCompilerUtils.cppm
        ShaderProgram.cppm
        PRIVATE FILE_SET shader_compiler_impl TYPE CXX_MODULES FILES
        private/Compiler.cppm
        private/CompileContext.cppm
        private/ConversionHelpers.cppm
        private/ShaderProgram.cppm
)

