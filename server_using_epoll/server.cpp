#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#define MAX_EVENTS 32 // max events to return by 'epoll_wait()'

// echo server, using 'epoll()' (multiplexing)

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

	struct sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(12345);
	sockAddr.sin_addr.s_addr = htons(INADDR_ANY);
	bind(masterSocket, (struct sockaddr *) &sockAddr, sizeof(sockAddr));

	// make 'accept' non-blocking
	set_nonblock(masterSocket);
	
	listen(masterSocket, SOMAXCONN);

	int epoll = epoll_create1(0);

	struct epoll_event event;
	event.data.fd = masterSocket;
	event.events = EPOLLIN; // wait until there is something to read
	epoll_ctl(epoll, EPOLL_CTL_ADD, masterSocket, &event);


	while (true)
	{
		struct epoll_event events[MAX_EVENTS];
		int n = epoll_wait(epoll, events, MAX_EVENTS, -1);

		for (size_t i = 0; i < n; ++i)
		{
			if (events[i].data.fd == masterSocket)
			{
				int clientSocket = accept(masterSocket, 0, 0);
				set_nonblock(clientSocket);
				struct epoll_event event;
				event.data.fd = clientSocket;
				event.events = EPOLLIN;
				epoll_ctl(epoll, EPOLL_CTL_ADD, clientSocket, &event);
			}
			else
			{
				static char buff[1024];
				int recvSize = recv(events[i].data.fd, buff, sizeof(buff) - 1, MSG_NOSIGNAL);
				if ((recvSize == 0) && (errno != EAGAIN))
				{
					shutdown(events[i].data.fd, SHUT_RDWR);
					close(events[i].data.fd);
				}
				else if (recvSize > 0)
				{
					send(events[i].data.fd, buff, recvSize, MSG_NOSIGNAL);
				}
			}
		}
	}

	return 0;
}
