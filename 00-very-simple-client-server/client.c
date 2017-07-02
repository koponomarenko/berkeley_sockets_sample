#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(12345);
    sockAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    connect(sock, (struct sockaddr *) &sockAddr, sizeof(sockAddr));

    char buffer[] = "PING";
    send(sock, buffer, sizeof(buffer) - 1, MSG_NOSIGNAL);
    recv(sock, buffer, sizeof(buffer) - 1, MSG_NOSIGNAL);

    shutdown(sock, SHUT_RDWR);
    close(sock);

    printf("%s\n", buffer);

    return 0;
}
