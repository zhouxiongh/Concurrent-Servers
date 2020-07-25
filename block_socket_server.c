//
// Created by Jason on 2020/7/25.
//

#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <zconf.h>
#include "utils.h"

int main(int argc, const char** argv) {
    setvbuf(stdout, NULL, _IONBF, 0);

    int port_num = 9090;
    if (argc >= 2) {
        port_num = atoi(argv[1]);
    }
    printf("Listening on port %d\n", port_num);

    int sock_fd = listen_inet_socket(port_num);
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len = sizeof(peer_addr);

    int new_sock_fd = accept(sock_fd, (struct sockaddr *) &peer_addr, &peer_addr_len);
    if (new_sock_fd < 0) {
        perror_die("error on accept");
    }
    report_peer_connected(&peer_addr, peer_addr_len);

    while (1) {
        uint8_t buf[1024];
        printf("Calling recv...\n");
        int len = recv(new_sock_fd, buf, sizeof(buf), 0);
        if (len < 0) {
            perror_die("recv");
        } else if (len == 0) {
            printf("Peer disconnected; i'm done.\n");
            break;
        }
        printf("recv returned %d bytes", len);
    }
    close(new_sock_fd);
    close(sock_fd);
    return 0;
}

