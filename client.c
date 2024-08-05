// // #include <stdio.h>
// // #include <stdlib.h>
// // #include <string.h>
// // #include <sys/socket.h>
// // #include <sys/types.h>
// // #include <unistd.h>
// // #include <dirent.h>
// // #include <time.h>

// // #include "controller.h"
// // #include "rawsocket.h"

// // #define WINDOW_SIZE 5
// // #define TIMEOUT 2 // Tempo em segundos
// // #define MAX_PAYLOAD_SIZE 63
// // #define MAX_RETRIES 10

// // int main() {
// //     printf("Iniciando cliente...\n");

// //     int sock = ConexaoRawSocket("lo");
// //     if (sock < 0) {
// //         perror("Erro ao criar o socket");
// //         return 1;
// //     }
// //     printf("Socket criado com sucesso\n");

// //     enviar_lista_arquivos(sock);

// //     char arquivo[63];
// //     printf("Digite o nome do vídeo que deseja baixar: ");
// //     scanf("%s", arquivo);
// //     printf("%s\n", arquivo);

// //     if (strlen(arquivo) > 63 || !is_valid_filename(arquivo)) {
// //         printf("Nome de arquivo inválido\n");
// //         return 1;
// //     }

// //     // printf("aui");
// //     // baixar_arquivo(sock, arquivo);

// //     // char caminho[1024];
// //     // snprintf(caminho, sizeof(caminho), "./%s", arquivo);
// //     // exibir_video(caminho);

// //     close(sock);
// //     return 0;
// // }

// #include <dirent.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <time.h>
// #include <unistd.h>

// #include "controller.h"
// #include "rawsocket.h"

// #define WINDOW_SIZE 5
// #define TIMEOUT 2  // Tempo em segundos
// #define MAX_PAYLOAD_SIZE 63
// #define MAX_RETRIES 10

// int main() {
//     printf("Iniciando cliente...\n");

//     int sock = ConexaoRawSocket("lo");
//     if (sock < 0) {
//         perror("Erro ao criar o socket");
//         return 1;
//     }
//     printf("Socket criado com sucesso\n");

//     // frame_t lista;

//     // lista.type = 0b01010;
//     // lista.init_mark = 126;

//     // send(sock, &lista, sizeof(frame_t), 0);

//     printf("Enviando lista\n");

//     frame_t videos;
//     frame_t resp_ack;

//     // recv(sock, &videos, sizeof(frame_t), 0);

//     // while (videos.init_mark != 126 || videos.type != 0b11110) {  // fim
//     //     if (videos.init_mark == 126 && videos.type == 0b10010) {
//     //         if (videos.crc == crc8(&videos)) {
//     //             printf("Disponível: %s\n", videos.data);
//     //             resp_ack.type = 0b00000;
//     //             resp_ack.init_mark = 126;
//     //             resp_ack.seq = videos.seq;
//     //             send(sock, &resp_ack, sizeof(frame_t), 0);
//     //         } else {
//     //             printf("Nack seq %d\n", videos.seq);
//     //             resp_ack.type = 0b00001;
//     //             resp_ack.init_mark = 126;
//     //             resp_ack.seq = videos.seq;
//     //             send(sock, &resp_ack, sizeof(frame_t), 0);
//     //         }
//     //     }
//     //     recv(sock, &videos, sizeof(frame_t), 0);
//     // }

//     // resp_ack.type = 0b11110;
//     // resp_ack.seq = videos.seq;
//     // send(sock, &resp_ack, sizeof(frame_t), 0);
//     // printf("enviei fim tx\n");

//     char arquivo[63];

//     printf("Digite o nome do arquivo que deseja enviar: ");
//     scanf("%s", arquivo);
//     printf("aaaa");

//     frame_t nome_video;
//     nome_video.type = 0b01011;
//     nome_video.init_mark = 126;
//     strncpy(nome_video.data, arquivo, 63);
//     printf("nome video: %s\n", nome_video.data);
//     nome_video.crc = crc8(&nome_video);
//     printf("cheguei aqui\n");
//     nome_video.size = strlen(nome_video.data);
//     printf("aq tbm\n");
//     nome_video.seq = 0;
//     printf("Enviando nome: %s", nome_video.data);
//     send(sock, &nome_video, sizeof(frame_t), 0);

//     // frame_t msg;

//     // while (1) {
//     //     if (recv(sock, &msg, sizeof(frame_t), 0) < 0) {
//     //         return 1;
//     //     }

//     //     if (msg.init_mark == 126 && msg.type == 0b10010) {
//     //         unsigned char received_crc = msg.crc;
//     //         unsigned char calculated_crc = crc8(&msg);

//     //         frame_t response_msg;
//     //         response_msg.init_mark = 0b01111110;
//     //         response_msg.size = 0;
//     //         response_msg.seq = msg.seq;
//     //         response_msg.crc = crc8(&msg);

//     //         if (received_crc == calculated_crc) {
//     //             printf("Recebi seq %d: %s\n", msg.seq, msg.data);
//     //             // fprintf(output_file, "%s", msg.data);
//     //             response_msg.type = 0b00000;  // ACK
//     //         } else {
//     //             printf("Erro de CRC na seq %d\n", msg.seq);
//     //             response_msg.type = 0b00001;  // NACK
//     //         }

//     //         if (send(sock, &response_msg, sizeof(frame_t), 0) < 0)
//     //             perror("Erro");
//     //     }
//     // }

//     close(sock);

//     return 0;
// }

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
                printf("Disponível: %s\n", videos.data);
                resp_ack.type = 0b00000;
                resp_ack.init_mark = 126;
                resp_ack.seq = videos.seq;
                send(sock, &resp_ack, sizeof(frame_t), 0);
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

    FILE* output_file = fopen(nome_video.data, "wb");
    if (!output_file) {
        perror("Erro ao abrir o arquivo para escrita");
    }

    frame_t msg;

    int seq;
    while (1) {
        if (recv(sock, &msg, sizeof(frame_t), 0) < 0) {
            return 1;
        }
        seq = 0;

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
                    printf("Recebi seq %d: %s\n", msg.seq, msg.data);
                    fwrite(msg.data, 1, msg.size, output_file);
                    // fprintf(output_file, "%s", msg.data);
                    response_msg.type = 0b00000;  // ACK
                    seq = (seq + 1) % 32;
                }
            } else {
                printf("Erro de CRC na seq %d\n", msg.seq);
                response_msg.type = 0b00001;  // NACK
            }

            if (send(sock, &response_msg, sizeof(frame_t), 0) < 0)
                perror("Erro");
        }
    }

    if (output_file) {
        fclose(output_file);
    }

    close(sock);
    return 0;
}
