add_executable(test-clear-add
                 test-clear-add.cpp
		 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

add_test(NAME test-clear-add COMMAND $<TARGET_FILE:test-clear-add>)

add_executable(test-clear-multiply
                 test-clear-multiply.cpp
		 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

add_test(NAME test-clear-multiply COMMAND $<TARGET_FILE:test-clear-multiply>)

add_executable(test-clear-subtract
                 test-clear-subtract.cpp
		 ${CMAKE_SOURCE_DIR}/src/circuit.cpp)

add_test(NAME test-clear-subtract COMMAND $<TARGET_FILE:test-clear-subtract>)

if (TFHE_FOUND)

    foreach(op IN ITEMS and or xor not)

        add_executable(test-tfhe-bool-${op}
                         test-tfhe-bool-${op}.cpp
                         ${CMAKE_SOURCE_DIR}/src/circuit.cpp
                         ${CMAKE_SOURCE_DIR}/src/simple-circuits.cpp)

        target_link_libraries(test-tfhe-bool-${op} ${TFHE_LIBRARIES})

        add_test(NAME test-tfhe-bool-${op} COMMAND $<TARGET_FILE:test-tfhe-bool-${op}>)

    endforeach(op)
endif (TFHE_FOUND)
