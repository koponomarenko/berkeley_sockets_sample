#include <iostream>
#include <set>
#include <algorithm>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#define POLL_SIZE 2048 // we need to set number of descriptors for poll

// echo server, using 'poll' (multiplexing)

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
	std::set<int> clientSockets;

	struct sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(12345);
	sockAddr.sin_addr.s_addr = htons(INADDR_ANY);
	bind(masterSocket, (struct sockaddr *) &sockAddr, sizeof(sockAddr));

	// make 'accept' non-blocking
	set_nonblock(masterSocket);
	
	listen(masterSocket, SOMAXCONN);

	struct pollfd sockSet[POLL_SIZE];
	sockSet[0].fd = masterSocket;
	sockSet[0].events = POLLIN; // wait for read events only

	while (true)
	{
		size_t index = 1;
		for (auto it = clientSockets.begin(); it != clientSockets.end(); ++it)
		{
			sockSet[index].fd = *it;
			sockSet[index].events = POLLIN;
		}
		size_t sockSetSize = 1 + clientSockets.size();

		poll(sockSet, sockSetSize, -1);

		for (size_t i = 0; i < sockSetSize; ++i)
		{
			if (sockSet[i].revents & POLLIN)
			{
				if (i) // event on client
				{
					static char buff[1024];
					int recvSize = recv(sockSet[i].fd, buff, sizeof(buff) - 1, MSG_NOSIGNAL);
					if ((recvSize == 0) && (errno != EAGAIN))
					{
						shutdown(sockSet[i].fd, SHUT_RDWR);
						close(sockSet[i].fd);
						clientSockets.erase(sockSet[i].fd);
					}
					else if (recvSize > 0)
					{
						send(sockSet[i].fd, buff, recvSize, MSG_NOSIGNAL);
					}
				}
				else // event on master
				{
					int clientSock = accept(masterSocket, 0, 0);
					set_nonblock(clientSock);
					clientSockets.insert(clientSock);
				}
			}
		}
	}

	return 0;
}
