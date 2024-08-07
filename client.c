

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "controller.h"
#include "rawsocket.h"

#define MAX_PAYLOAD_SIZE 63

int main() {
    printf("Iniciando cliente...\n");

    int sock = ConexaoRawSocket("lo");
    if (sock < 0) {
        perror("Erro ao criar o socket");
        return 1;
    }
    printf("Socket criado com sucesso\n");

    frame_t lista;

    lista.type = 0b01010;
    lista.init_mark = 126;

    send(sock, &lista, sizeof(frame_t), 0);

    printf("Enviando lista\n");

    frame_t videos;
    frame_t resp_ack;

    int seq = 0;

    while (1) {  // fim
        recv(sock, &videos, sizeof(frame_t), 0);
        if (videos.init_mark == 126 && videos.type == 0b11110) {
            resp_ack.type = 0b00000;
            resp_ack.init_mark = 126;
            resp_ack.seq = videos.seq;
            send(sock, &resp_ack, sizeof(frame_t), 0);
            break;
        }

        if (videos.init_mark == 126 && videos.type == 0b10010) {
            if (videos.crc == crc8(&videos)) {
                if (seq == videos.seq) {
                    printf("Disponível: %s\n", videos.data);
                    resp_ack.type = 0b00000;
                    resp_ack.init_mark = 126;
                    resp_ack.seq = videos.seq;
                    send(sock, &resp_ack, sizeof(frame_t), 0);
                    seq = (seq + 1) % 32;
                }
            } else {
                printf("Nack seq %d\n", videos.seq);
                resp_ack.type = 0b00001;
                resp_ack.init_mark = 126;
                resp_ack.seq = videos.seq;
                send(sock, &resp_ack, sizeof(frame_t), 0);
            }
        }
        recv(sock, &videos, sizeof(frame_t), 0);
    }

    // resp_ack.type = 0b11110;
    // resp_ack.seq = videos.seq;
    // send(sock, &resp_ack, sizeof(frame_t), 0);
    // printf("enviei fim tx\n");

    char arquivo[64];  // Aumente o tamanho para garantir que é 63 + null
                       // terminator

    printf("Digite o nome do vídeo que deseja baixar: ");
    scanf("%63s", arquivo);  // Limite a entrada para evitar overflow
    printf("Nome do arquivo: %s\n", arquivo);

    frame_t nome_video;
    nome_video.type = 0b01011;  // Tipo para enviar nome do vídeo
    nome_video.init_mark = 126;
    strncpy(nome_video.data, arquivo, MAX_PAYLOAD_SIZE);
    nome_video.crc = crc8(&nome_video);  // Calcule CRC
    nome_video.size = strlen(nome_video.data);
    nome_video.seq = 0;

    printf("Enviando nome: %s\n", nome_video.data);
    if (send(sock, &nome_video, sizeof(frame_t), 0) < 0) {
        perror("Erro ao enviar nome do vídeo");
        close(sock);
        return 1;
    }

    FILE *output_file = fopen(nome_video.data, "wb");
    if (!output_file) {
        perror("Erro ao abrir o arquivo para escrita");
    }

    frame_t msg;

    seq = 0;

    while (1){
        if (recv(sock, &msg, sizeof(frame_t), 0) < 0) {
            return 1;
        }
        if(msg.type != 0b11110){
            break;
        }
    }

    while (1) {
        if(msg.init_mark == 126 && msg.type == 0b11110 && msg.seq != 0){
            printf("recebi finalização\n");
            break;
        }
        

        if (msg.init_mark == 126 && msg.type == 0b10010) {
            unsigned char received_crc = msg.crc;
            unsigned char calculated_crc = crc8(&msg);

            frame_t response_msg;
            response_msg.init_mark = 0b01111110;
            response_msg.size = 0;
            response_msg.seq = msg.seq;
            response_msg.crc = crc8(&msg);

            if (received_crc == calculated_crc) {
                if (response_msg.seq == seq) {
                    // log_frame("client_log.txt", &msg, "");
                    // printf("Recebi seq %d: %s\n", msg.seq, msg.data);
                    // fwrite(msg.data, 1, msg.size, output_file);
                    // // fprintf(output_file, "%s", msg.data);
                    // response_msg.type = 0b00000;  // ACK
                    

                    unsigned char *deescaped_data;
                    size_t deescaped_size;

                    deescape_data((unsigned char *)msg.data, msg.size,
                                  &deescaped_data, &deescaped_size);

                    printf("Recebi seq %d: %s\n", msg.seq, msg.data);
                    fwrite(deescaped_data, 1, deescaped_size, output_file);
                    log_frame("client_log.txt", &msg, "Processado");

                    seq = (seq + 1) % 32;

                    free(deescaped_data);
                }
            } else {
                printf("Erro de CRC na seq %d\n", msg.seq);
                response_msg.type = 0b00001;  // NACK
            }

            if (send(sock, &response_msg, sizeof(frame_t), 0) < 0)
                perror("Erro");
        
        }
        
        if (recv(sock, &msg, sizeof(frame_t), 0) < 0) {
            return 1;
        }
    }

    

    if (output_file) {
        fclose(output_file);
    }

    close(sock);
    return 0;
}