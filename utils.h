//
// Created by Jason on 2020/7/11.
//

#ifndef ASYNC_SOCKET_SERVER_UTILS_H
#define ASYNC_SOCKET_SERVER_UTILS_H

void perror_die(char* msg);
int listen_inet_socket(int);
void report_peer_connected(const struct sockaddr_in*, socklen_t);

#endif //ASYNC_SOCKET_SERVER_UTILS_H
