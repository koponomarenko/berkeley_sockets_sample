CC=gcc

all: build_server build_client

build_server:
	$(CC) -o server.out server.c

build_client:
	$(CC) -o client.out client.c

clean:
	rm *.out &>/dev/null
