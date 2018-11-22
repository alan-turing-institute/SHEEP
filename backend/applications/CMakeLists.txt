####################################################################################
## CLEAR
####################################################################################

add_executable(circuit-parser
  examples/circuit-parser.cpp ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

add_executable(circuit-example
  examples/circuit-example.cpp ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

add_test(NAME circuit-example COMMAND $<TARGET_FILE:circuit-example>)

add_executable(simple-circuit-maker
  utilities/simple-circuit-maker.cpp ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

add_executable(bitonic-sorting-circuit
  examples/bitonic-sorting-circuit.cpp
  ${CMAKE_SOURCE_DIR}/src/circuit.cpp
  ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp
  ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)

add_executable(clear-context-example
  examples/clear-context-example.cpp ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

add_test(NAME clear-context-example COMMAND $<TARGET_FILE:clear-context-example>)

####################################################################################
## HElib
####################################################################################

if (HElib_FOUND)
  add_executable(helib-context-example
    examples/helib-context-example.cpp
    ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

  add_test(NAME helib-context-example COMMAND $<TARGET_FILE:helib-context-example>)

  target_link_libraries(helib-context-example
    ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})

  add_executable(bitonic-sort-helib
    examples/bitonic-sort-helib.cpp
    ${CMAKE_SOURCE_DIR}/src/circuit.cpp
    ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp
    ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)

  target_link_libraries(bitonic-sort-helib
    ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})

  add_test(NAME bitonic-sort-helib COMMAND bitonic-sort-helib)

endif(HElib_FOUND)

 if (TFHE_FOUND)
     add_executable(tfhe-example
                      examples/tfhe-example.cpp
 		     ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

     add_test(NAME tfhe-example COMMAND $<TARGET_FILE:tfhe-example>)

     target_link_libraries(tfhe-example
                             ${TFHE_LIBRARIES})


     target_link_libraries(tfhe_example
                             ${TFHE_LIBRARIES})

     add_executable(bitonic-sort-tfhe
 		   examples/bitonic-sort-tfhe.cpp
   		   ${CMAKE_SOURCE_DIR}/src/circuit.cpp
   		   ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp
   		   ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)
     target_link_libraries(bitonic-sort-tfhe ${TFHE_LIBRARIES})

     add_test(NAME bitonic-sort-tfhe COMMAND bitonic-sort-tfhe)

 endif(TFHE_FOUND)

 if (SEAL_FOUND)
     add_executable(seal-example
                      examples/seal-example.cpp
 		     ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
     add_test(NAME seal-example COMMAND $<TARGET_FILE:seal-example>)
     target_link_libraries(seal-example
                             ${SEAL_LIBRARIES})

     add_executable(seal-param-selector
                      utilities/seal-param-selector.cpp)
     target_link_libraries(seal-param-selector
                             ${SEAL_LIBRARIES}
 			    pthread)
 endif(SEAL_FOUND)


 if (LP_FOUND)
     add_executable(lp-context-example
                      examples/lp-context-example.cpp
                      ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
     add_test(NAME lp-context-example COMMAND $<TARGET_FILE:lp-context-example>)
     target_link_libraries(lp-context-example
                             ${LP_LIBRARIES} ${GMP_LIBRARIES})
     add_executable(lp-example
                      examples/lpplain.cpp
                      ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

     add_test(NAME lp-example COMMAND $<TARGET_FILE:lp-example>)

     target_link_libraries(lp-example
                             ${LP_LIBRARIES} ${GMP_LIBRARIES})

 endif(LP_FOUND)

####################################################################################
## SHEEP-CL
##
## The "sheep-cl" executable, to run a specified context on a given circuit.
####################################################################################

add_executable(sheep-cl utilities/sheep-cl.cpp ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

if (HElib_FOUND)
  target_link_libraries(sheep-cl ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
endif(HElib_FOUND)

if (TFHE_FOUND)
  target_link_libraries(sheep-cl ${TFHE_LIBRARIES})
endif(TFHE_FOUND)

if (SEAL_FOUND)
  target_link_libraries(sheep-cl ${SEAL_LIBRARIES})
endif(SEAL_FOUND)

if (LP_FOUND)
  target_link_libraries(sheep-cl ${LP_LIBRARIES} ${GMP_LIBRARIES})
endif(LP_FOUND)

add_test(NAME sheep-cl COMMAND $<TARGET_FILE:sheep-cl>)

####################################################################################
## "run-sheep-server"
##
### The "run-sheep-server" executable, a REST API with endpoints to specify circuits, HE libraries etc and run tests.
####################################################################################

add_executable(run-sheep-server
    	       server/run-sheep-server.cpp
               ${CMAKE_SOURCE_DIR}/src/sheep-server.cpp
               ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

target_link_libraries(run-sheep-server cpprestsdk::cpprest ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

if (HElib_FOUND)
  target_link_libraries(run-sheep-server ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
endif(HElib_FOUND)

if (TFHE_FOUND)
  target_link_libraries(run-sheep-server ${TFHE_LIBRARIES})
endif(TFHE_FOUND)

if (SEAL_FOUND)
  target_link_libraries(run-sheep-server ${SEAL_LIBRARIES})
endif(SEAL_FOUND)

if (LP_FOUND)
  target_link_libraries(run-sheep-server ${LP_LIBRARIES} ${GMP_LIBRARIES})
endif(LP_FOUND)
