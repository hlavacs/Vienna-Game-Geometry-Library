file(GLOB_RECURSE SHADER_SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.comp)

foreach(SHADER_SRC ${SHADER_SRC_LIST})
    get_filename_component(FILE_NAME ${SHADER_SRC} NAME_WE)
    set(SHADER_SPV ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME}.spv)

    add_custom_command(
        OUTPUT ${SHADER_SPV}
        COMMAND glslc ${SHADER_SRC} -o ${SHADER_SPV}
        DEPENDS ${SHADER_SRC}
        COMMENT "Compiling *.comp → *.spv"
    )
    list(APPEND SHADER_BINARIES ${SHADER_SPV})
endforeach(SHADER_SRC)

add_custom_target(
    compile_shader ALL
    DEPENDS ${SHADER_BINARIES}
)