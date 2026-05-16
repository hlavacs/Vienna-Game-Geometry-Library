set(SHADER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/comp.comp)
set(SHADER_SPV ${CMAKE_CURRENT_SOURCE_DIR}/comp.spv)

add_custom_command(
    OUTPUT ${SHADER_SPV}
    COMMAND glslc ${SHADER_SRC} -o ${SHADER_SPV}
    DEPENDS ${SHADER_SRC}
    COMMENT "Compiling comp.comp → comp.spv"
)

add_custom_target(compile_shader ALL
    DEPENDS ${SHADER_SPV}
)