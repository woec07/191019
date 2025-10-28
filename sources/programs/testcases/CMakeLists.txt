#####
# File to add all tests to CTest
#####
include(${CMAKE_TOP}/add_test.cmake)

######
# Add Tests
######

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/matmul/test_data/*.cpp" )
foreach(item ${files})
    string(REPLACE ".cpp" "" test_num ${item})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/matmul/test_data/test_data_" "" test_num ${test_num})
    set(test_name "matmul")
    add_test_ref(${test_name} ${test_num})
endforeach()

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/add/test_data/*.cpp" )
foreach(item ${files})
    string(REPLACE ".cpp" "" test_num ${item})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/add/test_data/test_data_" "" test_num ${test_num})
    set(test_name "add")
    add_test_ref(${test_name} ${test_num})
endforeach()

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/mul/test_data/*.cpp" )
foreach(item ${files})
    string(REPLACE ".cpp" "" test_num ${item})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/mul/test_data/test_data_" "" test_num ${test_num})
    set(test_name "mul")
    add_test_ref(${test_name} ${test_num})
endforeach()