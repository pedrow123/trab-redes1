#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "rawsocket.h"
#include "controller.h"


int main() {
    int sock = ConexaoRawSocket("lo");
    frame_t* msg;
    printf("Recebendo\n");
    while (1) {
        recv(sock, msg, sizeof(frame_t), 0);
        if(msg->init_mark == 126){
            printf("Recebi: %s\n", msg->data);
        } else {
            printf("Não recebi info correta\n");
        }


        // if (strcmp(arr, "Lembrando os heróis do passado") == 0) {
        //     printf("Recebi: %s\n", arr);
        // } else {
        //     printf("não esta igual\n");
        // }
    }
    return 0;
}