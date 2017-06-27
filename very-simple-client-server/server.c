#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
    int masterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(12345);
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(masterSocket, (struct sockaddr *) &sockAddr, sizeof(sockAddr));
    listen(masterSocket, SOMAXCONN);

    while (1)
    {
        int slaveSocket = accept(masterSocket, 0, 0);

        int buffer[5] = {0};
        recv(slaveSocket, buffer, sizeof(buffer) - 1, MSG_NOSIGNAL);
        send(slaveSocket, buffer, sizeof(buffer) - 1, MSG_NOSIGNAL);

        shutdown(slaveSocket, SHUT_RDWR);
        close(slaveSocket);

        printf("%s\n", buffer);
    }

    return 0;
}
