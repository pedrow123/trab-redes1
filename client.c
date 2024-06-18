#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "rawsocket.h"
#include "controller.h"


int main(){
    int sock = ConexaoRawSocket("lo");
    char* hino = "Lembrando os heróis do passado";

    frame_t* msg = malloc(sizeof(frame_t));
    msg->init_mark = 126;
    msg->size = strlen(hino);
    msg->data = hino;



    send(sock, msg, sizeof(msg), 0);
    printf("Enviando hino");
    return 0;
}