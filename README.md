## Socket Programming
The client will send null terminated ASCII strings to the server using TCP and the server will replywith the same null terminated string unless the string “PING” is sent, in which case the server will reply with the null terminated string “PONG”.

**Compiling**

`gcc -o client client.c pingpong.c`

`gcc -o server server.c pingpong.c`

**Running**

1) On a shell: `./server`
2) On a shell on another window: `./cilent 10000`

**Demo**
![Demo Image](/examples/demonstration.jpg)
