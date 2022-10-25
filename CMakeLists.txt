cmake_minimum_required(VERSION 3.5.1)

project(test_networking C CXX)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/nwkit")

add_executable(one_shot_server src/01_one_shot/one_shot_server.c)

add_executable(test_client2 src_client/test_client2.c)