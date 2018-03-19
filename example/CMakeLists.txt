add_executable(circuit-parser
                 circuit-parser.cpp
		 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

add_executable(circuit-example
                 circuit-example.cpp
		 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
add_test(NAME circuit-example COMMAND $<TARGET_FILE:circuit-example>)

add_executable(simple-circuit-maker
                 simple-circuit-maker.cpp
		 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)




add_executable(clear-context-example
                 clear-context-example.cpp
                 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

add_test(NAME clear-context-example COMMAND $<TARGET_FILE:clear-context-example>)




##if (HELIB_FOUND)
##    add_executable(helib-context-example
##                     helib-context-example.cpp
##                     ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
##
##    add_test(NAME helib-context-example COMMAND $<TARGET_FILE:helib-context-example>)
##
##    target_link_libraries(helib-context-example
##                            ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
##
##    add_executable(bitonic-sort-helib	       
##    		   bitonic-sort-helib.cpp
##  		   ${CMAKE_SOURCE_DIR}/src/circuit.cpp
##  		   ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp
##  		   ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)
##
##    target_link_libraries(bitonic-sort-helib
##			  ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
##
##    add_test(NAME bitonic-sort-helib COMMAND bitonic-sort-helib)
##
##
##
##endif(HELIB_FOUND)
##
##if (TFHE_FOUND)
##    add_executable(tfhe-example
##                     tfhe-example.cpp
##		     ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
##
##    add_test(NAME tfhe-example COMMAND $<TARGET_FILE:tfhe-example>)
##
##    target_link_libraries(tfhe-example
##                            ${TFHE_LIBRARIES})
##endif(TFHE_FOUND)
##
##add_executable(bitonic-sorting-circuit
##  bitonic-sorting-circuit.cpp
##  ${CMAKE_SOURCE_DIR}/src/circuit.cpp
##  ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp
##  ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)
##
##target_link_libraries(bitonic-sorting-circuit ${TFHE_LIBRARIES})
##
##add_test(NAME bitonic-sorting-circuit COMMAND bitonic-sorting-circuit)
##
##if (HELIB_FOUND AND TFHE_FOUND)
##    add_executable(benchmark		       
##    		  benchmark.cpp
##                  ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
##    target_link_libraries(benchmark		  
##		  		${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES} ${TFHE_LIBRARIES})
##
##    add_test(NAME benchmark COMMAND $<TARGET_FILE:benchmark>)
##
##endif(HELIB_FOUND AND TFHE_FOUND)
##
