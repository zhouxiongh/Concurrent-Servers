//
// Created by Jason on 2020/7/23.
//
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <zconf.h>
#include <pthread.h>
#include "utils.h"

typedef enum {
    WAIT_FOR_MSG,
    IN_MSG
} ProcessingState;
typedef struct { int sockfd; } thread_config_t;

void serve_connection(int);

void* server_thread(void* arg);


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

        int newsockfd =
                accept(sock_fd, (struct sockaddr*)&peer_addr, &peer_addr_len);

        if (newsockfd < 0) {
            perror_die("ERROR on accept");
        }

        report_peer_connected(&peer_addr, peer_addr_len);
        pthread_t the_thread;

        thread_config_t* config = (thread_config_t*)malloc(sizeof(*config));
        if (!config) {
            perror_die("OOM");
        }
        config->sockfd = newsockfd;
        pthread_create(&the_thread, NULL, server_thread, config);

        // Detach the thread - when it's done, its resources will be cleaned up.
        // Since the main thread lives forever, it will outlive the serving threads.
        pthread_detach(the_thread);
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

void* server_thread(void* arg) {
    thread_config_t* config = (thread_config_t *)arg;
    int sock_fd = config->sockfd;
    free(config);

    unsigned long id = (unsigned long)pthread_self();
    printf("Thread %lu created to handle connection with socket %d\n", id, sock_fd);
    serve_connection(sock_fd);
    printf("Thread %lu done\n", id);
    return 0;

}
