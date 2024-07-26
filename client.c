// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <unistd.h>

// #include "controller.h"
// #include "rawsocket.h"

// int main() {
//     int sock = ConexaoRawSocket("lo");
//     char* hino = "Lembrando os heróis do ";

//     frame_t msg;
//     msg.init_mark = 0b01111110;
//     msg.size = strlen(hino);
//     msg.seq = 0;         // Primeiro pacote
//     msg.type = 0b10010;  // Tipo dados
//     strncpy(msg.data, hino, 63);
//     msg.crc = crc8(&msg);

//     // printf("\n%d\n", msg->crc);

//     send(sock, &msg, sizeof(frame_t), 0);
//     printf("Enviando hino\n");

//     int retries = 0;

//     frame_t ack_msg;

//     while (1) {
//         if (recv(sock, &ack_msg, sizeof(frame_t), 0) > 0) {
//             if (ack_msg.init_mark == 126) {
//                 if (ack_msg.type == 0b00000) {
//                     printf("\nRECEBI O ACKKKK  \n");
//                     break;
//                 } else if(ack_msg.type == 0b00001) {
//                     printf("RECEBI NACKKKKK\n");
//                     // print_frame(ack_msg);
//                     char *teste = "Lembrando os heróis do passado!";
//                     strncpy(msg.data, teste, 63);
//                     msg.crc = crc8(&msg);
//                     msg.size = strlen(msg.data);
//                     retries++;
//                     send(sock, &msg, sizeof(frame_t), 0);

//                 }
//             }
//         }
//     }

//     // if(retries == 5)
//     //     printf("\nApós 5 tentativas falhou\n");
//     // free(msg);
//     close(sock);
//     return 0;
// }

// // #include <stdio.h>
// // #include <stdlib.h>
// // #include <sys/types.h>
// // #include <sys/socket.h>
// // #include <string.h>
// // #include <unistd.h>

// // #include "rawsocket.h"
// // #include "controller.h"

// // int main(){
// //     int sock = ConexaoRawSocket("lo");
// //     char* hino = "Lembrando os heróis do passado";

// //     frame_t* msg = malloc(sizeof(frame_t));
// //     msg->init_mark = 0b01111110;
// //     msg->size = strlen(hino);
// //     msg->seq = 0; // Primeiro pacote
// //     msg->type = 0b10010; // Tipo dados
// //     strncpy(msg->data, hino, 63);

// //     // Calcular o CRC8
// //     msg->crc = crc8(msg);

// //     frame_t ack_msg;
// //     int attempts = 0;
// //     int max_attempts = 5;
// //     int ack_received = 0;

// //     while (attempts < max_attempts && !ack_received) {
// //         send(sock, msg, sizeof(frame_t), 0);
// //         printf("Enviando hino\n");

// //         if (recv(sock, &ack_msg, sizeof(frame_t), 0) > 0) {
// //             if (ack_msg.init_mark == 126 && ack_msg.type == 0b00000 &&
// //             ack_msg.seq == msg->seq) {
// //                 printf("ACK recebido\n");
// //                 ack_received = 1;
// //             } else if (ack_msg.init_mark == 126 && ack_msg.type == 0b00001
// &&
// //             ack_msg.seq == msg->seq) {
// //                 printf("NACK recebido, reenviando...\n");
// //                 attempts++;
// //             }
// //         } else {
// //             printf("Timeout ou erro de recepção, reenviando...\n");
// //             attempts++;
// //         }
// //     }

// //     if (!ack_received) {
// //         printf("Falha ao enviar o frame após %d tentativas\n",
// max_attempts);
// //     }

// //     free(msg);
// //     close(sock);
// //     return 0;
// // }

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
#define MAX_RETRIES 5

int main() {
    int sock = ConexaoRawSocket("lo");
    char file_name[256];
    char file_path[512];

    printf("Arquivos disponíveis:\n");
    list_files("videos");  // Substitua pelo caminho do diretório onde estão os
                           // arquivos de texto

    printf("\nDigite o nome do arquivo que deseja transferir: ");
    scanf("%s", file_name);

    snprintf(file_path, sizeof(file_path), "%s%s", "videos/", file_name);

    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    frame_t msg;
    msg.init_mark = 0b01111110;
    msg.size = 0;
    msg.seq = 0;         // Primeiro pacote
    msg.type = 0b10010;  // Tipo dados

    int retries = 0;
    const int max_retries = 5;

    struct timeval tv;
    tv.tv_sec = 2;  // Timeout de 2 segundos
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    frame_t ack_msg;
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        size_t len = strlen(buffer);
        size_t offset = 0;

        while (offset < len) {
            size_t chunk_size = (len - offset) > MAX_PAYLOAD_SIZE
                                    ? MAX_PAYLOAD_SIZE
                                    : (len - offset);
            strncpy(msg.data, buffer + offset, chunk_size);
            msg.data[chunk_size] = '\0';
            msg.size = chunk_size;
            msg.crc = crc8(&msg);

            send(sock, &msg, sizeof(frame_t), 0);
            printf("Enviando seq %d: %s\n", msg.seq, msg.data);

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
                printf("\nFalha após %d tentativas para seq %d\n", MAX_RETRIES,
                       msg.seq);
                break;
            }

            offset += chunk_size;
            msg.seq++;
        }
    }

    fclose(file);
    close(sock);
    return 0;
}
