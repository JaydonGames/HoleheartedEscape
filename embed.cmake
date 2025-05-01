# Accepts formats LENGTH, WITH_EXT, NULL_DELIM
function(embed TARGET BINARY NAMESPACE TYPE FORMAT)
    
    string(COMPARE EQUAL "${FORMAT}" "WITH_EXT" WITH_EXT)
    if (WITH_EXT)
        get_filename_component(NAME "${BINARY}" NAME)
        string(REPLACE "." "_" NAME ${NAME})
    else()
        get_filename_component(NAME "${BINARY}" NAME_WE)
    endif()

    set(SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/embeded_assets")
    get_filename_component(SRC "${BINARY}" NAME)
    set(SRC "${SRC_DIR}/${SRC}.cpp")
    file(MAKE_DIRECTORY ${SRC_DIR})

    add_custom_command(
        DEPENDS "${BINARY}" "${CMAKE_SOURCE_DIR}/embed.cmake"
        OUTPUT "${SRC}"
        COMMAND ${CMAKE_COMMAND}
            -D BINARY="${BINARY}"
            -D SRC="${SRC}"
            -D NAMESPACE="${NAMESPACE}"
            -D NAME="${NAME}"
            -D TYPE="${TYPE}"
            -D RUN_EMBED=
            -D FORMAT="${FORMAT}"
            -P "${CMAKE_SOURCE_DIR}/embed.cmake"
    )
    target_sources(${TARGET} PRIVATE ${SRC})
endfunction()

if(DEFINED RUN_EMBED AND BINARY AND SRC AND NAMESPACE AND NAME AND TYPE AND FORMAT)
    string(REPLACE "\\ " " " BINARY "${BINARY}")
    string(REPLACE "\\ " " " SRC "${SRC}")
    string(REPLACE "\\ " " " TYPE "${TYPE}")

    file(READ ${BINARY} BINARY HEX)

    string(LENGTH "${BINARY}" BYTES)
    math(EXPR BYTES "${BYTES}/2")
    math(EXPR ARRSIZE "${BYTES}+1")

    string(REGEX REPLACE "\(..\)" ",0x\\1" BINARY ${BINARY})
    string(SUBSTRING "${BINARY}" 1 -1 BINARY)

    string(COMPARE EQUAL "${FORMAT}" "LENGTH" WITH_LENGTH)
    if (WITH_LENGTH)
        file(WRITE ${SRC} "namespace ${NAMESPACE} {\n\tstruct asset_t {\n\t\tconst unsigned long long length = ${BYTES};\n\t\t${TYPE} data[${ARRSIZE}]{${BINARY}, 0x0};\n\t};\n\textern asset_t ${NAME};\n\tasset_t ${NAME};\n}")
    else()
        file(WRITE ${SRC} "namespace ${NAMESPACE} {\n\textern ${TYPE} ${NAME}[]{${BINARY}, 0x0};\n}")
    endif()
endif()
