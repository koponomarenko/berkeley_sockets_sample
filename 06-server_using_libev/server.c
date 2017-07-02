#include <ev.h>

#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>


void read_cb(struct ev_loop * loop, struct ev_io * watcher, int revents)
{
	char buffer[1024];
	size_t r = recv(watcher->fd, buffer, 1024, MSG_NOSIGNAL);
	if (r < 0)
	{
		return;
	}
	else if (r == 0)
	{
		ev_io_stop(loop, watcher);
		free(watcher);
		return;
	}
	else
	{
		send(watcher->fd, buffer, r, MSG_NOSIGNAL);
	}
}

void accept_cb(struct ev_loop * loop, struct ev_io * watcher, int revents)
{
	int client_sd = accept(watcher->fd, 0, 0);

	struct ev_io * w_client = (struct ev_io *) malloc(sizeof(struct ev_io));
	ev_io_init(w_client, read_cb, client_sd, EV_READ);
	ev_io_start(loop, w_client);
}

int main(int agrc, char ** argv)
{
	struct ev_loop * loop = ev_default_loop(0);

	int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in sockAddr;
	bzero(&sockAddr, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(12345);
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(sd, (struct sockaddr *) &sockAddr, sizeof(sockAddr));

	listen(sd, SOMAXCONN);

	struct ev_io w_accept;
	ev_io_init(&w_accept, accept_cb, sd, EV_READ);
	ev_io_start(loop, &w_accept);

	while (1)
	{
		ev_loop(loop, 0);
	}

	return 0;
}
