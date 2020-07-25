//
// Created by Jason on 2020/7/25.
//
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <zconf.h>
#include <fcntl.h>
#include <errno.h>
#include "utils.h"

int main(int argc, const char** argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    int portnum = 9988;
    if (argc >= 2) {
        portnum = atoi(argv[1]);
    }
    printf("Listening on port %d\n", portnum);
    int sockfd = listen_inet_socket(portnum);
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len = sizeof(peer_addr);

    int newsockfd = accept(sockfd, (struct sockaddr*)&peer_addr, &peer_addr_len);
    if (newsockfd < 0) {
        perror_die("ERROR on accept");
    }
    report_peer_connected(&peer_addr, peer_addr_len);

    // Set nonblocking mode on the socket.
    make_socket_non_blocking(newsockfd);

    while (1) {
        uint8_t buf[1024];
        printf("Calling recv...\n");
        int len = recv(newsockfd, buf, sizeof(buf), 0);
        if (len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(200 * 1000);
                continue;
            }
            perror_die("recv");
        } else if (len == 0) {
            printf("Peer disconnected; i'm done.\n");
            break;
        }
        printf("recv returned %d bytes", len);
    }
}

