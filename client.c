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
    msg->init_mark = 0b01111110;
    msg->size = strlen(hino);
    strncpy(msg->data, hino, 63);

    printf("%ld", sizeof(frame_t));
    printf("%ld", sizeof(msg));
    send(sock, msg, sizeof(frame_t), 0);
    printf("Enviando hino");
    free(msg);
    return 0;
}