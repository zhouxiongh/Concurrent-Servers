//
// Created by Jason on 2020/7/25.
//

#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <zconf.h>
#include <sys/select.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include "utils.h"

#define MAXFDS 1000

typedef struct {
    bool want_read;
    bool want_write;
} fd_status_t;

typedef enum {
    INITAL_ACK,
    WAIT_FOR_MSG,
    IN_MSG
} ProcessingState;

#define SENDBUF_SIZE 1024

typedef struct {
    ProcessingState state;
    uint8_t sendbuf[SENDBUF_SIZE];
    int sendbuf_end;
    int sendptr;
} peer_state_t;

const fd_status_t fd_status_R = {.want_read = true, .want_write = false};
const fd_status_t fd_status_W = {.want_read = false, .want_write = true};
const fd_status_t fd_status_RW = {.want_read = true, .want_write = true};
const fd_status_t fd_status_NORW = {.want_read = false, .want_write = false};

peer_state_t global_state[MAXFDS];

fd_status_t on_peer_ready_send(int sockfd) {
    assert(sockfd < MAXFDS);
    peer_state_t* peerState = &global_state[sockfd];

    if (peerState->sendptr >= peerState->sendbuf_end) {
        return fd_status_RW;
    }
    int sendlen = peerState->sendbuf_end - peerState->sendptr;
    int nsent = send(sockfd, &peerState->sendbuf[peerState->sendptr], sendlen, 0);
    if (nsent == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return fd_status_W;
        } else {
            perror_die("send");
        }
    }
    if (nsent < sendlen) {
        peerState->sendptr += nsent;
        return fd_status_W;
    } else {
        peerState->sendptr = 0;
        peerState->sendbuf_end = 0;

        if (peerState->state == INITAL_ACK) {
            peerState->state = WAIT_FOR_MSG;
        }

        return fd_status_R;
    }
}

fd_status_t on_peer_ready_recv(int sockfd) {
    assert(sockfd < MAXFDS);
    peer_state_t* peerState = &global_state[sockfd];

    if (peerState->state == INITAL_ACK ||
        peerState->sendptr < peerState->sendbuf_end) {
        return fd_status_W;
    }

    uint8_t buf[1024];
    int nbytes = recv(sockfd, buf, sizeof(buf), 0);
    if (nbytes == 0) {
        return fd_status_NORW;
    } else if (nbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return fd_status_R;
        } else {
            perror_die("recv");
        }
    }
    bool ready_to_send = false;
    for (int i = 0; i < nbytes; i++) {
        switch(peerState->state) {
            case INITAL_ACK:
                assert(0 && "can't reach here");
                break;
            case WAIT_FOR_MSG:
                if (buf[i] == '^') {
                    peerState->state = IN_MSG;
                }
                break;;
            case IN_MSG:
                if (buf[i] == '$') {
                    peerState->state = WAIT_FOR_MSG;
                } else {
                    assert(peerState->sendbuf_end < SENDBUF_SIZE);
                    peerState->sendbuf[peerState->sendbuf_end++] = buf[i] + 1;
                    ready_to_send = true;
                }
                break;
        }
    }
    return (fd_status_t) {.want_read = !ready_to_send,
            .want_write = ready_to_send};
}

int main(int argc, const char** argv) {
    setvbuf(stdout, NULL, _IONBF, 0);

    int port_num = 9090;
    if(argc >= 2) {
        port_num = atoi(argv[1]);
    }
    int listener_sockfd = listen_inet_socket(port_num);

    // The "master" sets are owned by the loop, tracking which FDs we want to
    // monitor for reading and which FDs we want to monitor for writing.
    fd_set readfds_master;
    FD_ZERO(&readfds_master);
    fd_set writefds_master;
    FD_ZERO(&writefds_master);

    // The listenting socket is always monitored for read, to detect when new
    // peer connections are incoming.
    FD_SET(listener_sockfd, &readfds_master);

    int fdset_max = listener_sockfd;

    while (1) {
        fd_set readfds = readfds_master;
        fd_set writefds = writefds_master;

        int nready = select(fdset_max+1, &readfds, &writefds, NULL, NULL);
        if (nready < 0) {
            perror_die("select");
        }

        for (int fd = 0; fd <= fdset_max && nready > 0; fd++) {
            if (FD_ISSET(fd, &readfds)) {
                nready--;

                if (fd == listener_sockfd) {
                    // The listening socket is ready; this means a new peer is connecting.
                } else {
                    fd_status_t status = on_peer_ready_recv(fd);
                    status = on_peer_ready_send(fd);

                    if (status.want_read) {
                        FD_SET(fd, &readfds_master);
                    } else {
                        FD_CLR(fd, &readfds_master);
                    }
                    if (status.want_write) {
                        FD_SET(fd, &writefds_master);
                    } else {
                        FD_CLR(fd, &writefds_master);
                    }

                    if (!status.want_write && !status.want_read) {
                        printf("socker %d closing\n", fd);
                        close(fd);
                    }
                }
            }
        }
    }
}



