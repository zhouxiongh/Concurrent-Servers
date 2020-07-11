#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <zconf.h>
#include "utils.h"

typedef enum {
    WAIT_FOR_MSG,
    IN_MSG
} ProcessingState;

void serve_connection(int);


int main(int argc, char** argv) {

    setvbuf(stdout, NULL, _IONBF, 0);

    int port_num = 9090;

    if(argc >= 2) {
        port_num = atoi(argv[1]);
    }

    int sock_fd = listen_inet_socket(port_num);
    while (1) {
        struct sockaddr_in peer_addr;
        socklen_t peer_addr_len = sizeof(peer_addr);

        int newsockfd = accept(sock_fd, (struct sockaddr_in*)&peer_addr, &peer_addr_len);

        if (newsockfd < 0) {
            perror("ERROR on accept");
        }

        report_peer_connected(&peer_addr, peer_addr_len);
        serve_connection(newsockfd);
        printf("peer done\n");
    }
    return 0;
}

void serve_connection(int sockfd) {
    if (send(sockfd, "*", 1, 0) < 1) {
        perror("send");
    }

    ProcessingState state = WAIT_FOR_MSG;

    while (1) {
        uint8_t buf[1024];
        int len = recv(sockfd, buf, sizeof(buf), 0);
        if (len < 0) {
            perror_die("recv");
        } else if (len == 0) {
            break;
        }

        for (int i = 0; i < len; ++i) {
            switch (state) {
                case WAIT_FOR_MSG:
                    if (buf[i] == '^') {
                        state = IN_MSG;
                    }
                    break;
                case IN_MSG:
                    if (buf[i] == '$') {
                        state = WAIT_FOR_MSG;
                    } else {
                        buf[i] += 1;
                        if (send(sockfd, &buf[i], 1, 0) < 1) {
                            perror("send error");
                            close(sockfd);
                            return;
                        }
                    }
                    break;
            }
        }
    }
    close(sockfd);
}

