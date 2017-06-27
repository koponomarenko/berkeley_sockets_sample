#include <iostream>
#include <set>
#include <algorithm>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

// echo server, using 'select' (multiplexing)

int set_nonblock(int fd)
{
    int flags;
#ifdef O_NONBLOCK
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
    {
        flags = 0;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    flags = 1;
    return ioctl(fd, FIONBIO, &flags);
#endif
}

int main(int argc, char ** argv)
{
    int masterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    std::set<int> clientSockets; // holds connected clients

    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(12345);
    sockAddr.sin_addr.s_addr = htons(INADDR_ANY);
    bind(masterSocket, (struct sockaddr *) &sockAddr, sizeof(sockAddr));

    // make 'accept' non-blocking
    set_nonblock(masterSocket);

    listen(masterSocket, SOMAXCONN);

    while (true)
    {
        fd_set sockSet;
        FD_ZERO(&sockSet);
        FD_SET(masterSocket, &sockSet);
        for (auto it = clientSockets.begin(); it != clientSockets.end(); ++it)
        {
            FD_SET(*it, &sockSet);
        }

        // we dont want to spend time checking unused elements in the 'set'
        // max elements for 'select' is 1024 (1023)
        // new sockets always use the smallest available descriptor
        int max = std::max(masterSocket, *std::max_element(clientSockets.begin(), clientSockets.end()));

        // wait here to read something
        select(max + 1, &sockSet, nullptr, nullptr, nullptr);

        // select() triggered on clients
        for (auto it = clientSockets.begin(); it != clientSockets.end(); ++it)
        {
            if (FD_ISSET(*it, &sockSet))
            {
                static char buff[1024];
                int recvSize = recv(*it, buff, sizeof(buff) - 1, MSG_NOSIGNAL);
                if ((recvSize == 0) && (errno != EAGAIN))
                {
                    shutdown(*it, SHUT_RDWR);
                    close(*it);
                    clientSockets.erase(it);
                }
                else if (recvSize != 0)
                {
                    send(*it, buff, recvSize, MSG_NOSIGNAL);
                }
            }
        }

        // select() triggered on master (new client joined)
        if (FD_ISSET(masterSocket, &sockSet))
        {
            int clientSocket = accept(masterSocket, 0, 0);
            set_nonblock(clientSocket);
            clientSockets.insert(clientSocket);
        }
    }

    return 0;
}
