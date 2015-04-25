#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common.h"

#define BUFFER_SIZE 1024

static char buffer[BUFFER_SIZE];

#define MAXCLIENTS 30

int main(int argc, char *argv[])
{
    int srvsock;
    int cltsock[MAXCLIENTS];
    struct sockaddr_in srvaddr;
    //struct sockaddr_in cltaddr;
    socklen_t cltlen;
    int msglen;
    int status;
    int maxfd;
    fd_set rdfds;
    int nready;
    int i;

    memset(cltsock, 0, MAXCLIENTS * sizeof(int));

    srvsock = socket(AF_INET, SOCK_STREAM, 0);
    assert(srvsock > 0);
    maxfd = srvsock;

    bzero((char *) &srvaddr, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_addr.s_addr = INADDR_ANY;
    srvaddr.sin_port = htons(PORT);

    status = bind(srvsock, (struct sockaddr*)&srvaddr, sizeof(srvaddr));
    assert(status == 0);

    status = listen(srvsock, MAXCONN);
    assert(status == 0);

    while (1) {
        FD_ZERO(&rdfds);
        FD_SET(srvsock, &rdfds);

        for (i = 0; i < MAXCLIENTS; i++) {
            if (cltsock[i] > 0) {
                FD_SET(cltsock[i], &rdfds);
                if (cltsock[i] > maxfd) {
                    maxfd = cltsock[i];
                }
            }
        }

        nready = select(maxfd + 1, &rdfds, NULL, NULL, NULL);
        assert(nready >= 0);

        if (FD_ISSET(srvsock, &rdfds)) {
            int newsock = accept(srvsock, (struct sockaddr*)&srvaddr, &cltlen);
            assert(newsock > 0);

            printf("Client on socket: %d connected\n", newsock);

            for (i = 0; i < MAXCLIENTS; i++) {
                if (cltsock[i] == 0) {
                    cltsock[i] = newsock;
                    break;
                }
            }
        }

        for (i = 0; i < MAXCLIENTS; i++) {
            if (FD_ISSET(cltsock[i], &rdfds)) {
                memset(buffer, 0, BUFFER_SIZE);
                msglen = read(cltsock[i], buffer, BUFFER_SIZE);
                assert(msglen >= 0);
                if (msglen == 0) {
                    printf("Client on socket: %d disconnected\n", cltsock[i]);
                    close(cltsock[i]);
                    cltsock[i] = 0;
                } else {
                    printf("Client on socket: %d sent: %s\n", cltsock[i], buffer);
                    send(cltsock[i], buffer, strlen(buffer), 0);
                }
            }
        }
    }


    close(srvsock);

    return 0;
}

