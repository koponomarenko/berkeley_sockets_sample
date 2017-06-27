# A simple server for Linux using 'select()'

written in C++

Srever returns to the client whatever the client sends.
Server handles multiple connections - up to 1023 (limitation of 'select()').


DRAWBACKS:

Every time select() is called, the whole set (or array) of 'fd_set' structures is passed (copied) to the kernel, and then returned (copied again) back.
The server can only handle up to 1023 connected clients (limitation of 'select()').


TIPS:

It is possible to use 'telnet 127.0.0.1 12345' as a client to test this 'echo' server.
