


add_executable(test-write-read-circuit
                 test-write-read-circuit.cpp
		 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
add_test(NAME test-write-read-circuit COMMAND $<TARGET_FILE:test-write-read-circuit>)


foreach(op IN ITEMS add multiply subtract compare ripple-carry-add)
	  foreach(type IN ITEMS int8_t)
	  	       add_executable(test-clear-${type}-${op}	
                       test-clear-${type}-${op}.cpp
		       ${CMAKE_SOURCE_DIR}/src/circuit.cpp  
		       ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)			  
		       add_test(NAME test-clear-${type}-${op} COMMAND $<TARGET_FILE:test-clear-${type}-${op}>)
	  endforeach(type)
endforeach(op)


add_executable(test-bit-get-set test-bit-get-set.cpp)
add_test(NAME test-bit-get-set COMMAND $<TARGET_FILE:test-bit-get-set>)

if (TFHE_FOUND)
    add_executable(test-tfhe-bool-encrypt-decrypt
                     test-tfhe-bool-encrypt-decrypt.cpp)
    target_link_libraries(test-tfhe-bool-encrypt-decrypt ${TFHE_LIBRARIES})
    add_test(NAME test-tfhe-bool-encrypt-decrypt COMMAND $<TARGET_FILE:test-tfhe-bool-encrypt-decrypt>)

    foreach(op IN ITEMS and or xor not)
        add_executable(test-tfhe-bool-${op}
                         test-tfhe-bool-${op}.cpp
                         ${CMAKE_SOURCE_DIR}/src/circuit.cpp
                         ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)

        target_link_libraries(test-tfhe-bool-${op} ${TFHE_LIBRARIES})

        add_test(NAME test-tfhe-bool-${op} COMMAND $<TARGET_FILE:test-tfhe-bool-${op}>)
    endforeach(op)

    foreach(type IN ITEMS int8_t)
        add_executable(test-tfhe-${type}-encrypt-decrypt
                         test-tfhe-${type}-encrypt-decrypt.cpp)
        target_link_libraries(test-tfhe-${type}-encrypt-decrypt ${TFHE_LIBRARIES})
        add_test(NAME test-tfhe-${type}-encrypt-decrypt COMMAND $<TARGET_FILE:test-tfhe-${type}-encrypt-decrypt>)

        foreach(op IN ITEMS add subtract negate compare)
            add_executable(test-tfhe-${type}-${op}
                             test-tfhe-${type}-${op}.cpp
                             ${CMAKE_SOURCE_DIR}/src/circuit.cpp
                             ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)

            target_link_libraries(test-tfhe-${type}-${op} ${TFHE_LIBRARIES})

            add_test(NAME test-tfhe-${type}-${op} COMMAND $<TARGET_FILE:test-tfhe-${type}-${op}>)
        endforeach(op)
    endforeach(type)

endif (TFHE_FOUND)

if (HELIB_FOUND)
    add_executable(test-helib-uint8_t-encrypt-decrypt
                     test-helib-uint8_t-encrypt-decrypt.cpp
		     ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
    target_link_libraries(test-helib-uint8_t-encrypt-decrypt ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
    add_test(NAME test-helib-uint8_t-encrypt-decrypt COMMAND $<TARGET_FILE:test-helib-uint8_t-encrypt-decrypt>)

    foreach(op IN ITEMS add multiply subtract)
    	add_executable(test-helib-uint8_t-${op}
		test-helib-uint8_t-${op}.cpp	
		${CMAKE_SOURCE_DIR}/src/circuit.cpp)
	target_link_libraries(test-helib-uint8_t-${op} ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
	add_test(NAME test-helib-uint8_t-${op} COMMAND $<TARGET_FILE:test-helib-uint8_t-${op}>)	
    endforeach(op)
    add_executable(test-helib-int8_t-negate
	test-helib-int8_t-negate.cpp	
	${CMAKE_SOURCE_DIR}/src/circuit.cpp)
    target_link_libraries(test-helib-int8_t-negate ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
    add_test(NAME test-helib-int8_t-negate COMMAND $<TARGET_FILE:test-helib-int8_t-negate>)
    
endif (HELIB_FOUND)
