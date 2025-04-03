function(embed TARGET BINARY NAMESPACE TYPE NAME)
    set(SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/embeded_assets")
    file(MAKE_DIRECTORY ${SRC_DIR})
    set(SRC "${SRC_DIR}/${NAME}.cpp")

    add_custom_command(
        DEPENDS "${BINARY}" "${CMAKE_SOURCE_DIR}/embed.cmake"
        OUTPUT "${SRC}"
        COMMAND ${CMAKE_COMMAND}
            -D BINARY="${BINARY}"
            -D SRC="${SRC}"
            -D NAMESPACE="namespace ${NAMESPACE} {"
            -D DEFINITION="    extern ${TYPE} ${NAME}[] = {"
            -D LENGTHNAME="${NAME}_length"
            -D RUN_EMBED=
            -P "${CMAKE_SOURCE_DIR}/embed.cmake"
    )
    target_sources(${TARGET} PRIVATE ${SRC})
endfunction()

if(DEFINED RUN_EMBED AND BINARY AND SRC AND NAMESPACE AND DEFINITION AND LENGTHNAME)
    string(REPLACE "\\ " " " DEFINITION "${DEFINITION}")
    string(REPLACE "\\ " " " NAMESPACE "${NAMESPACE}")
    string(REPLACE "\\ " " " BINARY "${BINARY}")
    string(REPLACE "\\ " " " SRC "${SRC}")

    file(READ ${BINARY} BINARY HEX)

    string(LENGTH "${BINARY}" BYTES)
    math(EXPR BYTES "${BYTES}/2")

    string(REGEX REPLACE "\(..\)" ",0x\\1" BINARY ${BINARY})
    string(SUBSTRING "${BINARY}" 1 -1 BINARY)

    file(WRITE ${SRC} "${NAMESPACE}\n${DEFINITION}${BINARY}, 0x0};\n    extern const unsigned long long ${LENGTHNAME} = ${BYTES};\n}")
endif()
