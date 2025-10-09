cmake_minimum_required(VERSION 3.10)
message(STATUS "CMake Version ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")

##########################
#
# Configuration variables:
# - RV_ARCH : select the processor architecture configuration for compilation
#
#
#
##########################

#Set this on the command line to change configuration.  Currently Supported: rv32im, rv32imzve32x
set(RISCV_ARCH rv32im_zve32x CACHE STRING "Specify the configuration")
set(MEM_LATENCY 1 CACHE STRING "Memory latency")


#####
# Default Paths to Subdirectories
#####

set(BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(MODEL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../rtl/build)
set(BSP_TOP ${CMAKE_CURRENT_SOURCE_DIR}/bsp)
set(TOOLCHAIN_TOP ${CMAKE_CURRENT_SOURCE_DIR}/../../toolchain)
set(CMAKE_TOP ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
set(TEST_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/testcases)

set(STUDENT_TOP ${CMAKE_CURRENT_SOURCE_DIR}/../../student_files)
set(TESTCASES_TOP ${CMAKE_CURRENT_SOURCE_DIR}/testcases)
set(SOURCES_TOP ${CMAKE_CURRENT_SOURCE_DIR})


#Include user defined configuration data
include(${STUDENT_TOP}/vector_config.cmake)
set(MEM_W ${VMEM_W}) # Before Caches, ensure MEM_W=VMEM_W
message("Selected VREG_W = ${VREG_W}")
message("Selected VMEM_W = ${VMEM_W}")
message("Selected VPROC_PIPELINES = ${VPROC_PIPELINES}")
message("Selected MEM_W = ${MEM_W}")




#####
# Select RISCV_ARCH and RISCV_ABI and Autovectorization flags based on selected features
#####

#These control benchmark configurations specific to the selected RISCV_ARCH
option(RISCV_ZVE32X "Use the RISC-V Embedded V Extension" OFF)
set(RISCV_ABI ilp32)


set(AUTOVEC_FLAGS "")
 
if(${RISCV_ARCH} STREQUAL "rv32im_zve32x") 
    set(RISCV_ZVE32X ON)
    set(AUTOVEC_FLAGS "")

else()
    message(FATAL_ERROR "Unsupported RISCV_ARCH selected")
    
endif()

set(RISCV_CMODEL "medany" CACHE STRING "mcmodel argument to the compiler")


#####
#Set GCC as the Compiler
#####

# Tell CMake about our processor
set(CMAKE_SYSTEM_PROCESSOR ${RISCV_ARCH})
# The Generic system name is used for bare-metal targets (without OS) in CMake
set(CMAKE_SYSTEM_NAME Generic)


set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CMake/gcc_toolchain.cmake")

####
# Set build mode, Debug by default
####
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE DEBUG)
endif()

if(PRINT)
  add_definitions(-DPRINT_OUTPUTS)
endif()


####
# Set some compiler options for easier debug or faster builds
####

# See whats going in CMake while getting the build process up and running
set(CMAKE_VERBOSE_MAKEFILE ON)

# Using C and C++ 11
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O1 -g -ggdb ${AUTOVEC_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -g -ggdb ${AUTOVEC_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O1 ${AUTOVEC_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O1 ${AUTOVEC_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wshadow -Wno-format -std=c17 ${AUTOVEC_FLAGS}") 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 ${AUTOVEC_FLAGS}")


#####################
# Setup the CMake Project
####################

set(CMAKE_EXECUTABLE_SUFFIX_C .elf)
set(CMAKE_EXECUTABLE_SUFFIX_CXX .elf)
set(CMAKE_EXECUTABLE_SUFFIX_ASM .elf) 
set(CMAKE_EXECUTABLE_SUFFIX ".elf") #For some reason setting this works for ASM files but SUFFIX_ASM doesnt


project(vicuna_edu LANGUAGES C CXX ASM DESCRIPTION "Compilation and Test Environment for Advanced Computer Architecture")
include(CTest)

########
#Add Program Subdirectories
########

add_subdirectory(testcases testcases)
add_subdirectory(bsp bsp)
add_subdirectory(../../student_files/testcases student_tests)

