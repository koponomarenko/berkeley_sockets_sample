#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/event.h>

// echo server, using 'kqueue()' (multiplexing)


// MSG_NOSIGNAL is not defined on macOS
#ifndef MSG_NOSIGNAL
    #define MSG_NOSIGNAL 0
    #ifdef SO_NOSIGPIPE
        #define SETSOCKOPT_SO_NOSIGPIPE
    #else
        #error "Cannot block SIGPIPE!"
    #endif
#endif


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
#ifdef SETSOCKOPT_SO_NOSIGPIPE
    const int sockOptValue = 1;
    setsockopt(masterSocket, SOL_SOCKET, SO_NOSIGPIPE, &sockOptValue, sizeof(sockOptValue));
#endif

	struct sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(12345);
	sockAddr.sin_addr.s_addr = htons(INADDR_ANY);
	bind(masterSocket, (struct sockaddr *) &sockAddr, sizeof(sockAddr));

	// make 'accept' non-blocking
	set_nonblock(masterSocket);
	
	listen(masterSocket, SOMAXCONN);

    int kqueueFd = kqueue();

	struct kevent event;
    bzero(&event, sizeof(event));
    EV_SET(&event, masterSocket, EVFILT_READ, EV_ADD, 0, 0, 0);
    kevent(kqueueFd, &event, 1, nullptr, 0, nullptr);

	while (true)
	{
        bzero(&event, sizeof(event));
        kevent(kqueueFd, nullptr, 0, &event, 1, nullptr);

        if (event.filter == EVFILT_READ)
        {
            if (event.ident == masterSocket)
            {
                int clientSocket = accept(masterSocket, 0, 0);
#ifdef SETSOCKOPT_SO_NOSIGPIPE
                const int sockOptValue = 1;
                setsockopt(clientSocket, SOL_SOCKET, SO_NOSIGPIPE, &sockOptValue, sizeof(sockOptValue));
#endif
                set_nonblock(clientSocket);
                bzero(&event, sizeof(event));
                EV_SET(&event, clientSocket, EVFILT_READ, EV_ADD, 0, 0, 0);
                kevent(kqueueFd, &event, 1, nullptr, 0, nullptr);
            }
            else
            {
                char buff[1024];
                int recvSize = recv(event.ident, buff, sizeof(buff) - 1, MSG_NOSIGNAL);
                if (recvSize <= 0)
                {
                    shutdown(event.ident, SHUT_RDWR);
                    close(event.ident);
                    std::cout << "client disconnected" << std::endl;
                }
                else
                {
                    send(event.ident, buff, recvSize, MSG_NOSIGNAL);
                }
            }
        }
	}

	return 0;
}
