cmake_minimum_required(VERSION 3.16)
project(async_socket_server C)

set(CMAKE_C_STANDARD 99)

add_executable(seq_socket_server seq-server.c utils.c utils.h)

add_executable(thread_socket_server thread-server.c utils.c utils.h)

add_executable(block_socket_server block_socket_server.c utils.c utils.h)

add_executable(nonblock-socker-server nonblock-socker-server.c utils.c utils.h)

add_executable(select-server select-server.c utils.c utils.h)