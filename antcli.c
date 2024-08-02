#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>

#include "controller.h"
#include "rawsocket.h"

#define WINDOW_SIZE 5
#define TIMEOUT 2 // Tempo em segundos
#define MAX_PAYLOAD_SIZE 63
#define MAX_RETRIES 10

int main() {
    printf("Iniciando cliente...\n");

    int sock = ConexaoRawSocket("lo");
    if (sock < 0) {
        perror("Erro ao criar o socket");
        return 1;
    }
    printf("Socket criado com sucesso\n");

    char diretorio[256] = "videos";
    list_files(diretorio);

    char arquivo[63];
    printf("Digite o nome do arquivo que deseja enviar: ");
    scanf("%s", arquivo);

    char caminho[1024];
    snprintf(caminho, sizeof(caminho), "%s/%s", diretorio, arquivo);

    

    long file_size;
    unsigned char *buffer = read_file_to_buffer(caminho, &file_size);
    if (!buffer) {
        // close(sock);
        return 1;
    }

    printf("Arquivo lido com sucesso. Tamanho: %ld bytes\n", file_size);


    frame_t msg;
    msg.init_mark = 0b01111110;
    msg.seq = 0;         // Primeiro pacote
    msg.type = 0b10010;  // Tipo dados

    int retries = 0;
    int seq_nack;

    struct timeval tv; 
    tv.tv_sec = 2;  // Timeout de 2 segundos
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    frame_t ack_msg;
    frame_t sended_msg[WINDOW_SIZE];

    size_t offset = 0;

    int new_msgs = 0;
    int nack_received = 0;

    while (offset < file_size) {
        if(nack_received == 1){
            for(int i = 0; i < WINDOW_SIZE; i++){
                if(sended_msg[i].seq == seq_nack){
                    new_msgs = i+1;
                    send(sock, &sended_msg[i], sizeof(frame_t), 0);
                } else if (sended_msg[i].seq > seq_nack){
                    send(sock, &sended_msg[i], sizeof(frame_t), 0);
                }
            }
        }

        for (int i = new_msgs; i < WINDOW_SIZE; i++){

            size_t bytes_to_send = (file_size - offset > MAX_PAYLOAD_SIZE) ? MAX_PAYLOAD_SIZE : (file_size - offset);
            memcpy(msg.data, buffer + offset, bytes_to_send);
            msg.size = bytes_to_send;
            msg.crc = crc8(&msg);

            send(sock, &msg, sizeof(frame_t), 0); 
            printf("Enviando seq %d: %.*s\n", msg.seq, (int)bytes_to_send, msg.data);
            sended_msg[i] = msg;

            offset += bytes_to_send;
            msg.seq++;
        }

        if(recv(sock, &ack_msg, sizeof(frame_t), 0) > 0){
            if (ack_msg.init_mark == 126) {
                if (ack_msg.type == 0b00000) {
                    printf("\nRECEBI O ACK para seq %d\n", msg.seq);
                    new_msgs = 0;
                    nack_received = 0;
                    break;
                } else if (ack_msg.type == 0b00001) {
                    printf("RECEBI NACK para seq %d\n", msg.seq);
                    nack_received = 1;
                    // retries++;
                    seq_nack = ack_msg.seq;
                    // send(sock, &msg, sizeof(frame_t), 0); 
                }
            }
        }

    }

    free(buffer);

    close(sock);

    return 0;
}