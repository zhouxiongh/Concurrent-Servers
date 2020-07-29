//
// Created by Jason on 2020/7/11.
//

#ifndef ASYNC_SOCKET_SERVER_UTILS_H
#define ASYNC_SOCKET_SERVER_UTILS_H


#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

void die(char* fmt, ...);

void* xmalloc(size_t i);

void perror_die(char* msg);
int listen_inet_socket(int);
void report_peer_connected(const struct sockaddr_in* sa, socklen_t salen);
void make_socket_non_blocking(int);

#endif //ASYNC_SOCKET_SERVER_UTILS_H
