#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#define UNIX_SOCKET_PATH "/tmp/echo-server.sock"


static void echo_read_cb(struct bufferevent * bev, void * ctx)
{
	struct evbuffer * input = bufferevent_get_input(bev);
	struct evbuffer * output = bufferevent_get_output(bev);

	size_t len = evbuffer_get_length(input);
	char * data = malloc(len);
	evbuffer_copyout(input, data, len);
	//data[len - 1] = '\0';
	printf("data: %s", data);
	free(data);

	evbuffer_add_buffer(output, input);
}

static void echo_event_cb(struct bufferevent * bev, short events, void * ctx)
{
	if (events & BEV_EVENT_ERROR)
	{
		perror("Error");
		bufferevent_free(bev);
	}
	if (events & BEV_EVENT_EOF)
	{
		bufferevent_free(bev);
	}
}

static void accept_conn_cb(struct evconnlistener * listener, evutil_socket_t fd,
	struct sockaddr * addr, int addrLen, void * ctx)
{
	struct event_base * base = evconnlistener_get_base(listener);
	struct bufferevent * bev = bufferevent_socket_new(base, fd,
		BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);
	bufferevent_enable(bev, EV_READ | EV_WRITE);
}

static void accept_error_cb(struct evconnlistener * listener, void * ctx)
{
	struct event_base * base = evconnlistener_get_base(listener);
	int err = EVUTIL_SOCKET_ERROR();
	fprintf(stderr, "Error %d: %s\n", err, evutil_socket_error_to_string(err));
	event_base_loopexit(base, NULL);
}

int main(int argc, char ** argv)
{
	unlink(UNIX_SOCKET_PATH);

	struct event_base * base = event_base_new();

	struct sockaddr_un sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sun_family = AF_UNIX;
	strcpy(sockAddr.sun_path, UNIX_SOCKET_PATH);

	struct evconnlistener * listener = evconnlistener_new_bind(base, accept_conn_cb,
		NULL, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, SOMAXCONN,
		(struct sockaddr *) &sockAddr, sizeof(sockAddr));
	evconnlistener_set_error_cb(listener, accept_error_cb);

	event_base_dispatch(base);

	return 0;
}
