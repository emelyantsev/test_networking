cmake_minimum_required(VERSION 3.5.1)

project(test_networking C CXX)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/nwkit")

add_executable(test_client2 src_client/test_client2.c)

add_executable(one_shot_server src/01_one_shot/one_shot_server.c)
add_executable(q_server src/02_queueing/q_server.c)


add_executable(thread_server src/03_threading/thread_server.cpp)

if (UNIX)
    target_link_libraries( thread_server pthread )
    set(CMAKE_CXX_COMPILER /bin/g++-10)
endif (UNIX)

add_executable(select_server src/05_multiplexing/select_server.cpp src/05_multiplexing/tcp_server.cpp)

add_library(nwkit src/nwkit/nwkit.cpp src/nwkit/coro_tools.cpp)
set_property(TARGET nwkit PROPERTY CXX_STANDARD 20)
if (UNIX)
target_compile_options(nwkit PUBLIC "-fcoroutines")
endif (UNIX)

add_executable(coro_server src/06_coroutine/coro_server.cpp src/06_coroutine/socket_handlers.cpp)
target_link_libraries(coro_server nwkit)
set_property(TARGET coro_server PROPERTY CXX_STANDARD 20)

if (UNIX)
target_compile_options(coro_server PUBLIC "-fcoroutines")
endif (UNIX)