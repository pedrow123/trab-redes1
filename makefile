# Alvo principal: compilar todos os binários
all : client server

# Compilar rawsocket.o
rawsocket.o : rawsocket.h

# Compilar controller.o
controller.o : controller.h

# Compilar server.o a partir de server.c, rawsocket.h, controller.h
server.o : server.c rawsocket.h controller.h

# Compilar client.o a partir de client.c, rawsocket.h, controller.h
client.o : client.c rawsocket.h controller.h

# Linkar server.o, rawsocket.o, e controller.o para criar o binário do servidor
server : server.o rawsocket.o controller.o

# Linkar client.o, rawsocket.o, e controller.o para criar o binário do cliente
client : client.o rawsocket.o controller.o

# Limpar arquivos de objetos e binários
clean :
	rm -f *.o server client
