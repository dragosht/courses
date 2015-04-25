#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common.h"

#define BUFFER_SIZE 1024

char buffer[BUFFER_SIZE];

int main(int argc, char* argv[])
{
    int sock;
    struct sockaddr_in srvaddr;
    int msglen;
    int status;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock > 0);

    bzero((char*) &srvaddr, sizeof(srvaddr) );
    srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(PORT);

    status = connect(sock, (struct sockaddr*) &srvaddr, sizeof(srvaddr));
    assert(status == 0);

    while (1) {
        printf("Enter message:\n");
        scanf("%s", buffer);

        msglen = send(sock, buffer, strlen(buffer), 0);
        assert(msglen >= 0);

        memset(buffer, 0, BUFFER_SIZE);
        msglen = recv(sock, buffer, BUFFER_SIZE, 0);
        assert(msglen >= 0);

        printf("Server reply:\n");
        puts(buffer);
    }

    close(sock);
    return 0;
}

