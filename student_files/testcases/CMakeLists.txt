#####
# File to add all tests to CTest
#####
include(${CMAKE_TOP}/add_test.cmake)

######
# Add Tests
######
file(GLOB files "${TESTCASES_TOP}/matmul/test_data/*.cpp" )
foreach(item ${files})
    string(REPLACE ".cpp" "" test_num ${item})
    string(REPLACE "${TESTCASES_TOP}/matmul/test_data/test_data_" "" test_num ${test_num})
    set(test_name "matmul")
    add_test_student(${test_name} ${test_num})
    comparison(${test_name} ${test_num})
endforeach()

file(GLOB files "${TESTCASES_TOP}/add/test_data/*.cpp" )
foreach(item ${files})
    string(REPLACE ".cpp" "" test_num ${item})
    string(REPLACE "${TESTCASES_TOP}/add/test_data/test_data_" "" test_num ${test_num})
    set(test_name "add")
    add_test_student(${test_name} ${test_num})
    comparison(${test_name} ${test_num})
endforeach()