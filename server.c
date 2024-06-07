#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "rawsocket.h"

int main(){
    int sock = ConexaoRawSocket("lo");
    char arr[256];
    printf("Recebendo\n");
    while(1){
        recv(sock, arr, sizeof(arr), 0);
        printf("Recebi: %s\n", arr);
    }
    return 0;
}