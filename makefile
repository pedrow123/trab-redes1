
all : client server

rawsocket.o : rawsocket.h
server.o : server.c rawsocket.h
client.o : client.c rawsocket.h

server : server.o rawsocket.o 
client : client.o rawsocket.o