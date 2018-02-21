add_executable(circuit-parser
                 circuit-parser.cpp
		 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

add_executable(circuit-example
                 circuit-example.cpp
		 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

add_test(NAME circuit-example COMMAND $<TARGET_FILE:circuit-example>)


add_executable(clear-context-example
                 clear-context-example.cpp
                 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

add_test(NAME clear-context-example COMMAND $<TARGET_FILE:clear-context-example>)


if (HELIB_FOUND)
    add_executable(helib-context-example
                     helib-context-example.cpp
                     ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

    add_test(NAME helib-context-example COMMAND $<TARGET_FILE:helib-context-example>)

    target_link_libraries(helib-context-example
                            ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
endif(HELIB_FOUND)

if (TFHE_FOUND)
    add_executable(tfhe-example
                     tfhe-example.cpp
		     ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

    add_test(NAME tfhe-example COMMAND $<TARGET_FILE:tfhe-example>)

    target_link_libraries(tfhe-example
                            ${TFHE_LIBRARIES})
endif(TFHE_FOUND)
