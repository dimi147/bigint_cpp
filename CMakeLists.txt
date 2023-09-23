cmake_minimum_required(VERSION 3.0.0)
project(bigint VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(bigint bigint.cpp)
add_executable(bigint_test bigint.cpp bigint_test.cpp)

install(TARGETS bigint_test bigint
        CONFIGURATIONS Debug
        DESTINATION ${CMAKE_SOURCE_DIR}/build/install)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
