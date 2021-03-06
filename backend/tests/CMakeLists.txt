set(CTEST_OUTPUT_ON_FAILURE TRUE)

# # Flags for profiling
# set (CMAKE_C_FLAGS " -pg ")
# set (CMAKE_CXX_FLAGS " -pg ")

add_executable(test-invalid-circuit
  test-invalid-circuit.cpp
  ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
add_test(NAME test-invalid-circuit COMMAND $<TARGET_FILE:test-invalid-circuit>)

#add_executable(test-invalid-circuit-from-file
#  test-invalid-circuit-from-file.cpp
#  ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
#
#configure_file(good.sheep good.sheep)
#configure_file(multiple-input.sheep multiple-input.sheep)
#configure_file(multiple-assignment.sheep multiple-assignment.sheep)
#configure_file(undef-gate-input.sheep undef-gate-input.sheep)
#configure_file(undef-output.sheep undef-output.sheep)
#add_test(NAME test-invalid-circuit-from-file COMMAND $<TARGET_FILE:test-invalid-circuit-from-file>)

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

##################################################################################################
### CLEAR
##################################################################################################

 foreach(type IN ITEMS bool int8_t uint8_t)
     foreach(op IN ITEMS add compare multiply select subtract addConst rotate)
         add_executable(test-clear-${type}-${op}
             test-clear-${type}-${op}.cpp
             ${CMAKE_SOURCE_DIR}/src/circuit.cpp
             ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp
             ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)

         add_test(NAME test-clear-${type}-${op} COMMAND $<TARGET_FILE:test-clear-${type}-${op}>)
     endforeach(op)
 endforeach(type)


 foreach(type IN ITEMS double complex)
     foreach(op IN ITEMS add multiply) # rotate)
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

##################################################################################################
### TFHE
##################################################################################################

if (TFHE_FOUND)
  ## operations for bool plaintext
  foreach(op IN ITEMS and or xor not select subtract rotate encrypt-decrypt encrypt-serialize)
    add_executable(test-tfhe-bool-${op}
      test-tfhe-bool-${op}.cpp
      ${CMAKE_SOURCE_DIR}/src/circuit.cpp
      ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp
      ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)

    target_link_libraries(test-tfhe-bool-${op} ${TFHE_LIBRARIES})

    add_test(NAME test-tfhe-bool-${op} COMMAND $<TARGET_FILE:test-tfhe-bool-${op}>)
  endforeach(op)

  ## operations for int / uint plaintext
  foreach(type IN ITEMS int8_t uint8_t)

    foreach(op IN ITEMS add subtract multiply negate select compare rotate encrypt-decrypt encrypt-serialize)

      add_executable(test-tfhe-${type}-${op} test-tfhe-${type}-${op}.cpp ${CMAKE_SOURCE_DIR}/src/circuit.cpp
			  ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)

      target_link_libraries(test-tfhe-${type}-${op} ${TFHE_LIBRARIES})

      add_test(NAME test-tfhe-${type}-${op} COMMAND $<TARGET_FILE:test-tfhe-${type}-${op}>)
    endforeach(op)

  endforeach(type)

  # The test of seven gates
  add_executable(test-tfhe-bool-c7 test-tfhe-bool-c7 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
  target_link_libraries(test-tfhe-bool-c7 ${TFHE_LIBRARIES})
  add_test(NAME test-tfhe-bool-c7 COMMAND $<TARGET_FILE:test-tfhe-bool-c7>)

endif (TFHE_FOUND)

##################################################################################################
### HELIB
##################################################################################################
# for helib, we have "f2" (binary plaintext space) and "fp" (modulo-p).
# "COMPARE" is only implemented for f2. "ADDCONST", "MULTBYCONST" only for fp.  Everything else for both.

if (HELIB_FOUND)

    # input types
    foreach(type IN ITEMS bool int8_t uint8_t)
        # modules
        foreach(f IN ITEMS fp)
              # operations - ADDCONST, MULTBYCONST only for fp
	      foreach (op IN ITEMS addConst multByConst rotate)
        	           add_executable(test-helib-${f}-${type}-${op}
        		                test-helib-${f}-${type}-${op}
        		   ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
			   target_link_libraries(test-helib-${f}-${type}-${op} ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
                 	   add_test(NAME test-helib-${f}-${type}-${op} COMMAND $<TARGET_FILE:test-helib-${f}-${type}-${op}>)
              endforeach(op)
     	endforeach(f)

      foreach(f IN ITEMS f2)
        # COMPARE only for f2
	      foreach (op IN ITEMS compare)
        	           add_executable(test-helib-${f}-${type}-${op}
        		                test-helib-${f}-${type}-${op}
        		   ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
			   target_link_libraries(test-helib-${f}-${type}-${op} ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
                 	   add_test(NAME test-helib-${f}-${type}-${op} COMMAND $<TARGET_FILE:test-helib-${f}-${type}-${op}>)
              endforeach(op)
     	endforeach(f)

	    ## And now all the other gates, for both fp and f2
      foreach(f IN ITEMS fp f2)
        # operations
	      foreach (op IN ITEMS encrypt-decrypt negate add multiply subtract select encrypt-serialize)
        	           add_executable(test-helib-${f}-${type}-${op}
        		                test-helib-${f}-${type}-${op}
        		   ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
			   target_link_libraries(test-helib-${f}-${type}-${op} ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
                 	   add_test(NAME test-helib-${f}-${type}-${op} COMMAND $<TARGET_FILE:test-helib-${f}-${type}-${op}>)
              endforeach(op)
     	endforeach(f)
    endforeach(type)

  # finally, one more test, only for f2/bool
  add_executable(test-helib-f2-bool-maximum test-helib-f2-bool-maximum.cpp ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
  target_link_libraries(test-helib-f2-bool-maximum ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
  add_test(NAME test-helib-f2-bool-maximum COMMAND $<TARGET_FILE:test-helib-f2-bool-maximum>)

  # The test of seven gates
  foreach(fx IN ITEMS f2 fp)
    add_executable(test-helib-${fx}-bool-c7 test-helib-${fx}-bool-c7 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
    target_link_libraries(test-helib-${fx}-bool-c7 ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
    add_test(NAME test-helib-${fx}-bool-c7 COMMAND $<TARGET_FILE:test-helib-${fx}-bool-c7>)
  endforeach(fx)

#  add_executable(test-helib-fp-int8_t-encrypt-serialize
#    test-helib-fp-int8_t-encrypt-serialize.cpp
#    ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
#  target_link_libraries(test-helib-fp-int8_t-encrypt-serialize ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
#  add_test(NAME test-helib-fp-int8_t-encrypt-serialize COMMAND $<TARGET_FILE:test-helib-fp-int8_t-encrypt-serialize>)

endif (HELIB_FOUND)

##################################################################################################
### SEAL
##################################################################################################
if (SEAL_FOUND)

    foreach(type IN ITEMS bool int8_t uint8_t)
      foreach (op IN ITEMS add multiply subtract multByConst rotate negate)
          add_executable(test-seal-bfv-${type}-${op}
                          test-seal-bfv-${type}-${op}
        ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
        target_link_libraries(test-seal-bfv-${type}-${op} ${SEAL_LIBRARIES})
        add_test(NAME test-seal-bfv-${type}-${op} COMMAND $<TARGET_FILE:test-seal-bfv-${type}-${op}>)
      endforeach(op)
    endforeach(type)
    add_executable(test-seal-bfv-int64_t-rotate test-seal-bfv-int64_t-rotate ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
    target_link_libraries(test-seal-bfv-int64_t-rotate ${SEAL_LIBRARIES})
    add_test(NAME test-seal-bfv-int64_t-rotate COMMAND $<TARGET_FILE:test-seal-bfv-int64_t-rotate>)

  # The test of seven gates
  add_executable(test-seal-bfv-bool-c7 test-seal-bfv-bool-c7 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
  target_link_libraries(test-seal-bfv-bool-c7 ${SEAL_LIBRARIES})
  add_test(NAME test-seal-bfv-bool-c7 COMMAND $<TARGET_FILE:test-seal-bfv-bool-c7>)

### SEAL CKKS
  foreach(type IN ITEMS double complex )
    foreach (op IN ITEMS encrypt-decrypt add)
     add_executable(test-seal-ckks-${type}-${op}
   	test-seal-ckks-${type}-${op}.cpp
     ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
     target_link_libraries(test-seal-ckks-${type}-${op} ${SEAL_LIBRARIES})
     add_test(NAME test-seal-ckks-${type}-${op} COMMAND test-seal-ckks-${type}-${op})
    endforeach(op)
  endforeach(type)

  add_executable(test-seal-ckks-double-multiply
	test-seal-ckks-double-multiply.cpp
  ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
  target_link_libraries(test-seal-ckks-double-multiply ${SEAL_LIBRARIES})
  add_test(NAME test-seal-ckks-double-multiply COMMAND test-seal-ckks-double-multiply)

endif (SEAL_FOUND)

##################################################################################################
### LIBPAILLIER
##################################################################################################

if (LP_FOUND)
  foreach(type IN ITEMS bool int8_t uint8_t)
    foreach (op IN ITEMS encrypt-decrypt add multByConst negate rotate encrypt-serialize)
      add_executable(test-lp-${type}-${op} test-lp-${type}-${op} ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
      target_link_libraries(test-lp-${type}-${op} ${LP_LIBRARIES} ${GMP_LIBRARIES})
      add_test(NAME test-lp-${type}-${op} COMMAND $<TARGET_FILE:test-lp-${type}-${op}>)
    endforeach(op)
  endforeach(type)

  # The test of seven gates
  add_executable(test-lp-bool-c7 test-lp-bool-c7 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
  target_link_libraries(test-lp-bool-c7 ${LP_LIBRARIES} ${GMP_LIBRARIES})
  add_test(NAME test-lp-bool-c7 COMMAND $<TARGET_FILE:test-lp-bool-c7>)
endif (LP_FOUND)
############### end of libpaillier ###############################################################

#add_executable(test-parallel-eval test-parallel-eval.cpp
#  ${CMAKE_SOURCE_DIR}/src/circuit.cpp
#  ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp
#  ${CMAKE_SOURCE_DIR}/src/circuit-util.cpp)
#if (HELIB_FOUND AND TFHE_FOUND)
#   target_link_libraries(test-parallel-eval ${TFHE_LIBRARIES} ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
#elseif (HELIB_FOUND)
#   target_link_libraries(test-parallel-eval ${HElib_LIBRARIES} ${NTL_LIBRARIES} ${GMP_LIBRARIES})
#elseif (TFHE_FOUND)
#   target_link_libraries(test-parallel-eval ${TFHE_LIBRARIES} )
#else(HELIB_FOUND_AND_TFHE_FOUND)
#   target_link_libraries(test-parallel-eval)
#endif(HELIB_FOUND AND TFHE_FOUND)

# add_test(NAME test-parallel-eval COMMAND test-parallel-eval)


##################################################################################################
### PALISADE
##################################################################################################

if (PALISADE_FOUND)
  foreach (type IN ITEMS int8_t)
    foreach (op IN ITEMS encrypt-decrypt add)
      add_executable(test-palisade-${type}-${op} test-palisade-${type}-${op}
        ${CMAKE_SOURCE_DIR}/src/circuit.cpp)
      target_link_libraries(test-palisade-${type}-${op} ${PALISADE_LIBRARIES})
      add_test(NAME test-palisade-${type}-${op} COMMAND $<TARGET_FILE:test-palisade-${type}-${op}>)
    endforeach (op)
  endforeach (type)
endif (PALISADE_FOUND)

##################################################################################################
