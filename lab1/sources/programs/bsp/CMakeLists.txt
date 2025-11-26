
####
#Add bsp library for Vicuna
####
add_library(BSP_Vicuna STATIC)

target_include_directories(BSP_Vicuna PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(BSP_Vicuna PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/crt0.S
  ${CMAKE_CURRENT_SOURCE_DIR}/uart.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/uart.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vicuna_crt.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vicuna_crt.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/terminate_benchmark.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/terminate_benchmark.cpp
)


