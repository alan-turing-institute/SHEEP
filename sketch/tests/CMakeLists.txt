add_executable(test-write-read-circuit
                 test-write-read-circuit.cpp
		 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
add_test(NAME test-write-read-circuit COMMAND $<TARGET_FILE:test-write-read-circuit>)

add_executable(test-circuit-copy
  test-circuit-copy.cpp
  ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
add_test(NAME test-circuit-copy COMMAND test-circuit-copy)

add_executable(test-circuit-par
  test-circuit-par.cpp
  ${CMAKE_SOURCE_DIR}/src/circuit.cpp
  ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp)
add_test(NAME test-circuit-par COMMAND test-circuit-par)

add_executable(test-circuit-seq
  test-circuit-seq.cpp
  ${CMAKE_SOURCE_DIR}/src/circuit.cpp
  ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp)
add_test(NAME test-circuit-seq COMMAND test-circuit-seq)

foreach(type IN ITEMS bool int8_t uint8_t)		
	     foreach(op IN ITEMS add multiply subtract compare select)
	  	       add_executable(test-clear-${type}-${op}	
                       test-clear-${type}-${op}.cpp
		       ${CMAKE_SOURCE_DIR}/src/circuit.cpp
		       ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp
		       ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)
		       add_test(NAME test-clear-${type}-${op} COMMAND $<TARGET_FILE:test-clear-${type}-${op}>)
	  endforeach(op)
endforeach(type)
foreach(type IN ITEMS int8_t uint8_t)		
	  foreach(op IN ITEMS ripple-carry-add)
	  	       add_executable(test-clear-${type}-${op}	
                       test-clear-${type}-${op}.cpp
		       ${CMAKE_SOURCE_DIR}/src/circuit.cpp
		       ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp
		       ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)
		       add_test(NAME test-clear-${type}-${op} COMMAND $<TARGET_FILE:test-clear-${type}-${op}>)
	  endforeach(op)
endforeach(type)


add_executable(test-bit-get-set test-bit-get-set.cpp)
add_test(NAME test-bit-get-set COMMAND $<TARGET_FILE:test-bit-get-set>)

if (TFHE_FOUND)
    add_executable(test-tfhe-bool-encrypt-decrypt
                     test-tfhe-bool-encrypt-decrypt.cpp)
    target_link_libraries(test-tfhe-bool-encrypt-decrypt ${TFHE_LIBRARIES})
    add_test(NAME test-tfhe-bool-encrypt-decrypt COMMAND $<TARGET_FILE:test-tfhe-bool-encrypt-decrypt>)

    foreach(op IN ITEMS and or xor not select)
        add_executable(test-tfhe-bool-${op}
                         test-tfhe-bool-${op}.cpp
                         ${CMAKE_SOURCE_DIR}/src/circuit.cpp
			 ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp
                         ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)

        target_link_libraries(test-tfhe-bool-${op} ${TFHE_LIBRARIES})

        add_test(NAME test-tfhe-bool-${op} COMMAND $<TARGET_FILE:test-tfhe-bool-${op}>)
    endforeach(op)

    foreach(type IN ITEMS int8_t)
        add_executable(test-tfhe-${type}-encrypt-decrypt
                         test-tfhe-${type}-encrypt-decrypt.cpp)
        target_link_libraries(test-tfhe-${type}-encrypt-decrypt ${TFHE_LIBRARIES})
        add_test(NAME test-tfhe-${type}-encrypt-decrypt COMMAND $<TARGET_FILE:test-tfhe-${type}-encrypt-decrypt>)

        foreach(op IN ITEMS add subtract negate compare select)
            add_executable(test-tfhe-${type}-${op}
                             test-tfhe-${type}-${op}.cpp
                             ${CMAKE_SOURCE_DIR}/src/circuit.cpp
			     ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp
                             ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)

            target_link_libraries(test-tfhe-${type}-${op} ${TFHE_LIBRARIES})

            add_test(NAME test-tfhe-${type}-${op} COMMAND $<TARGET_FILE:test-tfhe-${type}-${op}>)
        endforeach(op)
    endforeach(type)

endif (TFHE_FOUND)

if (HELIB_FOUND)

    foreach(type IN ITEMS bool int8_t uint8_t)
        foreach(f IN ITEMS fp)
    	foreach (op IN ITEMS addConst multByConst)
        	add_executable(test-helib-${f}-${type}-${op}	
    	                  	test-helib-${f}-${type}-${op}
			 	${CMAKE_SOURCE_DIR}/src/circuit.cpp)
    		target_link_libraries(test-helib-${f}-${type}-${op} ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
    		add_test(NAME test-helib-${f}-${type}-${op} COMMAND $<TARGET_FILE:test-helib-${f}-${type}-${op}>)
    	endforeach(op)      
    	endforeach(f)
	foreach(f IN ITEMS f2)
        foreach(op IN ITEMS compare)	
    		add_executable(test-helib-${f}-${type}-${op}
			test-helib-${f}-${type}-${op}.cpp	
			${CMAKE_SOURCE_DIR}/src/circuit.cpp)
		target_link_libraries(test-helib-${f}-${type}-${op} ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
		add_test(NAME test-helib-${f}-${type}-${op} COMMAND $<TARGET_FILE:test-helib-${f}-${type}-${op}>)	
        endforeach(op)
    	endforeach(f)
    	foreach(f IN ITEMS f2 fp)
        foreach(op IN ITEMS add multiply select encrypt-decrypt negate subtract)	
    		add_executable(test-helib-${f}-${type}-${op}
			test-helib-${f}-${type}-${op}.cpp	
			${CMAKE_SOURCE_DIR}/src/circuit.cpp)
		target_link_libraries(test-helib-${f}-${type}-${op} ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
		add_test(NAME test-helib-${f}-${type}-${op} COMMAND $<TARGET_FILE:test-helib-${f}-${type}-${op}>)	
        endforeach(op)
    	endforeach(f)
    endforeach(type)

endif (HELIB_FOUND)

add_executable(test-parallel-eval test-parallel-eval.cpp
  ${CMAKE_SOURCE_DIR}/src/circuit.cpp
  ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp
  ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp)
if (HELIB_FOUND AND TFHE_FOUND)
   target_link_libraries(test-parallel-eval ${TFHE_LIBRARIES} ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
elseif (HELIB_FOUND)
   target_link_libraries(test-parallel-eval ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
elseif (TFHE_FOUND)
   target_link_libraries(test-parallel-eval ${TFHE_LIBRARIES} )
else(HELIB_FOUND_AND_TFHE_FOUND)
   target_link_libraries(test-parallel-eval)
endif(HELIB_FOUND AND TFHE_FOUND)  

add_test(NAME test-parallel-eval COMMAND test-parallel-eval)
