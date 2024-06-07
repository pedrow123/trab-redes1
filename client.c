#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "rawsocket.h"

int main(){
    int sock = ConexaoRawSocket("lo");
    char* hino = "Lembrando os heróis do passado";
    send(sock, hino, strlen(hino)+1, 0);
    printf("Enviando hino");
    return 0;
}