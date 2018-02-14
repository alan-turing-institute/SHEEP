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

