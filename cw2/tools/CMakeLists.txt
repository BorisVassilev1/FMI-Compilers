cmake_minimum_required(VERSION 3.16)
project(cool-tools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(SRC_DIR "${PROJECT_ROOT}/src")
set(INCLUDE_DIR "${PROJECT_ROOT}/include")
set(GEN_DIR "${PROJECT_ROOT}/gen")
set(LIB_DIR ${PROJECT_ROOT}/../cw1/lib/)
set(BUILD_DIR "${PROJECT_ROOT}/build")
set(SEMANTICS_DIR "${SRC_DIR}/semantics")
set(CODEGEN_DIR "${SRC_DIR}/codegen")
set(DRIVERS_DIR "${SRC_DIR}/drivers")

set(LEXER_LIB ${PROJECT_ROOT}/../cw1/build/libCoolLexer.a)

set(COMPILER_FLAGS 
	-g -Wall -Wextra -Wpedantic -std=c++26
	#-fconcepts-diagnostics-depth=4
	-DROOT_DIR="${PROJECT_ROOT}" -fPIC
)

add_subdirectory(${LIB_DIR}/sdp_2023 sdp_2023_build)

add_executable(parser 
	${DRIVERS_DIR}/ParserDriver.cpp
	${LIB_DIR}/sdp_2023/Regex/regexParser.cpp
)
target_link_libraries(parser PRIVATE lang ${LEXER_LIB})
target_include_directories(parser PRIVATE ${LIB_DIR}/sdp_2023/)
set_target_properties(parser PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BUILD_DIR}")
target_compile_options(parser PRIVATE ${COMPILER_FLAGS})
