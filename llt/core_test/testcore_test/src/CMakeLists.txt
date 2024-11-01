cmake_minimum_required(VERSION 3.14)
set(PROJ_NAME testcore_test)
project(${PROJ_NAME})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ROOT_DIR ${CMAKE_SOURCE_DIR}/../../../../../)

# test framework
set(TEST_DIR ${ROOT_DIR}/test)
set(GOOGLETEST_DIR ${TEST_DIR}/3rdparty/googletest)
set(GTEST_INC_DIR ${GOOGLETEST_DIR}/googletest/include)
set(MOCKCPP_DIR ${TEST_DIR}/3rdparty/mockcpp)
set(MOCKCPP_INC_DIR ${MOCKCPP_DIR}/include)

# source code
set(SRC_DIR ${ROOT_DIR}/src)
set(API_DIR ${SRC_DIR}/api)
set(INC_DIR ${SRC_DIR}/inc)
set(LIB_DIR ${SRC_DIR}/lib)
set(TESTCORE_DIR ${SRC_DIR}/core/testcore)
aux_source_directory(${LIB_DIR} SRC_LIST)
aux_source_directory(${TESTCORE_DIR} SRC_LIST)
aux_source_directory(. SRC_LIST)

include_directories(${API_DIR} ${INC_DIR} ${LIB_DIR} ${TESTCORE_DIR} ${GTEST_INC_DIR}
                    ${MOCKCPP_INC_DIR})

add_executable(${PROJ_NAME})
target_sources(${PROJ_NAME} PRIVATE ${SRC_LIST})

target_link_libraries(${PROJ_NAME} gtest mockcpp)

add_subdirectory(${GOOGLETEST_DIR} googletest)
add_subdirectory(${MOCKCPP_DIR} mockcpp)