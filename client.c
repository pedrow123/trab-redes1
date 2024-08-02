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

    struct timeval tv; 
    tv.tv_sec = 2;  // Timeout de 2 segundos
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    frame_t ack_msg;

    size_t offset = 0;
    while (offset < file_size) {
        size_t bytes_to_send = (file_size - offset > MAX_PAYLOAD_SIZE) ? MAX_PAYLOAD_SIZE : (file_size - offset);
        memcpy(msg.data, buffer + offset, bytes_to_send);
        msg.size = bytes_to_send;
        msg.crc = crc8(&msg);

        send(sock, &msg, sizeof(frame_t), 0); 
        printf("Enviando seq %d: %.*s\n", msg.seq, (int)bytes_to_send, msg.data);

        retries = 0;
        while (retries < MAX_RETRIES) {
            if (recv(sock, &ack_msg, sizeof(frame_t), 0) > 0) {
                if (ack_msg.init_mark == 126) {
                    if (ack_msg.type == 0b00000) {
                        printf("\nRECEBI O ACK para seq %d\n", msg.seq);
                        break;
                    } else if (ack_msg.type == 0b00001) {
                        printf("RECEBI NACK para seq %d\n", msg.seq);
                        retries++;
                        send(sock, &msg, sizeof(frame_t), 0); 
                    }
                }
            } else {
                // Timeout ou erro de recepção
                retries++;
                send(sock, &msg, sizeof(frame_t), 0); 
            }
        }

        if (retries == MAX_RETRIES) {
            printf("\nFalha após %d tentativas para seq %d\n", MAX_RETRIES, msg.seq);
            break;
        }

        offset += bytes_to_send;
        msg.seq++;
    }

    free(buffer);

    close(sock);

    return 0;
}