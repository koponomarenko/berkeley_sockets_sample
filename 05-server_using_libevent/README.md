# A simple server for Linux using 'poll()'

written in C++

Srever returns to the client whatever the client sends.
Server handles multiple connections - up to whatever you set.


DRAWBACKS:

Every time poll() is called, the whole set (or array) of 'pollfd' structures is passed (copied) to the kernel, and then returned (copied again) back.


TIPS:

It is possible to use 'telnet 127.0.0.1 12345' as a client to test this 'echo' server.
