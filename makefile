
all : client server

rawsocket.o : rawsocket.h
controller.o : controller.h
server.o : server.c rawsocket.h controller.h
client.o : client.c rawsocket.h controller.h

server : server.o rawsocket.o controller.o
client : client.o rawsocket.o controller.o