# A simple server for Linux using 'libevent'

written in C

Srever returns to the client whatever the client sends.
Server handles multiple connections - up to whatever you set.

This approach easily allows 10K connections on one machine.


TIPS:

Because this server uses Unix (file) sosket, you can use 'socat' (similar to netcat) as a client.
The command might look like: echo "bla" | socat - UNIX-CONNECT:/tmp/echo-server.sock
