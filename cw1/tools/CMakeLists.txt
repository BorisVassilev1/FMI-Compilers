cmake_minimum_required(VERSION 3.16)
project(cool-tools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(SRC_DIR "${PROJECT_ROOT}/src")
set(GEN_DIR "${PROJECT_ROOT}/gen")
set(BUILD_DIR "${PROJECT_ROOT}/build")
set(DRIVERS_DIR "${SRC_DIR}/drivers")
set(LIB_DIR "${PROJECT_ROOT}/lib")

add_executable(lexer ${DRIVERS_DIR}/LexerDriver.cpp)

add_subdirectory(${LIB_DIR}/sdp_2023 sdp_2023_build)

target_link_libraries(lexer PRIVATE lang)
target_include_directories(lexer PRIVATE
	${LIB_DIR}/sdp_2023/
)

set_target_properties(lexer PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${BUILD_DIR}"
)

target_compile_options(lexer PRIVATE 
	-g -Wall -Wextra -Wpedantic -std=c++26
	#-fconcepts-diagnostics-depth=4
	-DROOT_DIR="${PROJECT_ROOT}"
	-stdlib=libstdc++
)
