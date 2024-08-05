#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MAX_PAYLOAD_SIZE 63

#include "controller.h"

unsigned char crc8(frame_t *frame) {
    unsigned char gerador = 0x07;  // Polinômio gerador
    unsigned char crc = 0x00;      // Inicialmente 0

    for (unsigned int i = 0; i < frame->size; i++) {
        crc ^= frame->data[i];  // XOR do byte de dados com o CRC atual

        for (unsigned int j = 0; j < 8; j++) {
            if (crc & 0x80)  // Se o bit mais significativo (MSB) for 1
            {
                crc = (crc << 1) ^ gerador;  // Desloca para a esquerda e aplica
                                             // o polinômio gerador
            } else {
                crc <<= 1;  // Apenas desloca para a esquerda
            }
        }
    }

    return crc;
}

void print_frame(frame_t frame) {
    printf("%d - crc\n", frame.crc);
    printf("%s - data\n", frame.data);
    printf("%d - marcador inicio\n", frame.init_mark);
    printf("%d - sequencia\n", frame.seq);
    printf("%d - tam\n", frame.size);
    printf("%d - tipo\n", frame.type);
}

static void aguarda_confirmacao(int sock, frame_t frame) {
    frame_t response;

    if (recv(sock, &response, sizeof(frame_t), 0) < 0) {
        printf("Erro\n");
        return;
    }

    while (response.init_mark != 126 || response.seq != frame.seq ||
           response.type != 0b00000) {
        // se for nack
        if (response.init_mark == 126 && response.type == 0b00001) {
            send(sock, &frame, sizeof(frame_t), 0);
        }

        if (recv(sock, &response, sizeof(frame_t), 0) < 0) {
            printf("Erro\n");
            return;
        }
    }
}

void list_files(const char *path, int sock) {

    frame_t envio;
    int seq = 0;
    struct dirent *entry;
    DIR *dp = opendir(path);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dp))) {
        if (entry->d_type == DT_REG) {
            printf("%s\n", entry->d_name);
            strncpy((char *)envio.data, entry->d_name, MAX_PAYLOAD_SIZE);
            envio.seq = seq;
            envio.init_mark = 126;
            envio.size = strlen(envio.data);
            envio.type = 0b10010; //dados
            envio.crc = crc8(&envio);

            send(sock, &envio, sizeof(frame_t), 0);
            seq = (seq + 1) % 32;

            aguarda_confirmacao(sock, envio);
        }
    }

    frame_t fim;
    fim.init_mark = 126;
    fim.type = 0b11110;
    fim.seq = seq;
    send(sock, &fim, sizeof(frame_t), 0);

    aguarda_confirmacao(sock, fim);

    closedir(dp);
}

unsigned char *read_file_to_buffer(const char *filename, long *file_size) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *buffer = (unsigned char *)malloc(*file_size);
    if (!buffer) {
        perror("Erro ao alocar memória");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, *file_size, file);
    fclose(file);

    return buffer;
}

int is_valid_filename(const char *filename) {
    while (*filename) {
        if (*filename < 0x20 || *filename > 0x7E) {
            return 0;
        }
        filename++;
    }
    return 1;
}



// void enviar_arquivo(int sock, const char *arquivo) {
//     char caminho[1024];
//     snprintf(caminho, sizeof(caminho), "videos/%s", arquivo);

//     long file_size;
//     unsigned char *buffer = read_file_to_buffer(caminho, &file_size);
//     if (!buffer) {
//         frame_t error_msg;
//         error_msg.init_mark = 0b01111110;
//         error_msg.type = 0b11111;  // Tipo erro
//         error_msg.seq = 0;
//         error_msg.size = 1;
//         error_msg.data[0] = 2;  // Erro: não encontrado
//         error_msg.crc = crc8(&error_msg);
//         send(sock, &error_msg, sizeof(frame_t), 0);
//         return;
//     }

//     frame_t msg;
//     msg.init_mark = 0b01111110;
//     msg.seq = 0;
//     msg.type = 0b10010;  // Tipo dados

//     size_t offset = 0;
//     while (offset < file_size) {
//         size_t bytes_to_send = (file_size - offset > MAX_PAYLOAD_SIZE)
//                                    ? MAX_PAYLOAD_SIZE
//                                    : (file_size - offset);
//         memcpy(msg.data, buffer + offset, bytes_to_send);
//         msg.size = bytes_to_send;
//         msg.crc = crc8(&msg);
//         send(sock, &msg, sizeof(frame_t), 0);

//         frame_t ack_msg;
//         if (recv(sock, &ack_msg, sizeof(frame_t), 0) > 0) {
//             if (ack_msg.init_mark == 126 && ack_msg.type == 0b00000) {  // ACK
//                 offset += bytes_to_send;
//                 msg.seq++;
//             } else if (ack_msg.type == 0b00001) {      // NACK
//                 send(sock, &msg, sizeof(frame_t), 0);  // Reenviar
//             }
//         }
//     }

//     // Enviar mensagem de fim de transmissão
//     msg.type = 0b11110;  // Tipo fim tx
//     msg.size = 0;
//     msg.crc = crc8(&msg);
//     send(sock, &msg, sizeof(frame_t), 0);

//     free(buffer);
// }

// void listar_videos(int sock) {
//     int seq = 0;
//     frame_t msg;
//     msg.init_mark = 0b01111110;
//     msg.type = 0b10010;  // Tipo lista

//     struct dirent *entry;
//     DIR *dp = opendir("videos");
//     if (dp == NULL) {
//         perror("Erro ao abrir o diretório");
//         return;
//     }

//     while ((entry = readdir(dp))) {
//         if (entry->d_type == DT_REG) {
//             char *ext = strrchr(entry->d_name, '.');
//             if (ext && (strcmp(ext, ".mp4") == 0 || strcmp(ext, ".avi") == 0)) {
//                 strncpy((char *)msg.data, entry->d_name, MAX_PAYLOAD_SIZE);
//                 msg.size = strlen((char *)msg.data);
//                 msg.seq = seq;

//                 msg.crc = crc8(&msg);
//                 printf("Enviando lista: %s\n", msg.data);
//                 send(sock, &msg, sizeof(frame_t), 0);

//                 aguarda_confirmacao(sock, msg, seq);

//                 seq = (seq + 1) % 32;
//             }
//         }
//     }

//     closedir(dp);

//     printf("Enviei fim tx\n");
//     // Enviar mensagem de fim de transmissão da lista
//     msg.type = 0b11110;  // Tipo fim tx
//     msg.size = 0;
//     msg.crc = crc8(&msg);
//     send(sock, &msg, sizeof(frame_t), 0);

//     aguarda_confirmacao(sock, msg, seq);

// }

// void baixar_arquivo(int sock, char *arquivo) {
//     frame_t msg;
//     msg.init_mark = 0b01111110;
//     msg.type = 0b01011;  // Tipo baixar
//     msg.seq = 0;
//     strncpy(msg.data, arquivo, MAX_PAYLOAD_SIZE);
//     msg.size = strlen(msg.data);
//     msg.crc = crc8(&msg);
//     printf("mensagem: %s", msg.data);
//     send(sock, &msg, sizeof(frame_t), 0);

//     FILE *output_file = fopen("teste.txt", "wb");
//     if (!output_file) {
//         perror("Erro ao criar o arquivo de saída");
//         return;
//     }

//     frame_t response;
//     while (1) {
//         if (recv(sock, &response, sizeof(frame_t), 0) > 0) {
//             if (response.init_mark == 126) {
//                 if (response.type == 0b10010) {
//                     fwrite(response.data, 1, response.size, output_file);
//                     msg.type = 0b00000;  // ACK
//                 } else if (response.type == 0b11110) {
//                     break;
//                 }
//                 msg.seq = response.seq;
//                 msg.crc = crc8(&msg);
//                 send(sock, &msg, sizeof(frame_t), 0);
//             }
//         }
//     }

//     fclose(output_file);
// }

// void exibir_video(const char *caminho) {
//     char comando[1024];
//     snprintf(comando, sizeof(comando), "mpv %s", caminho);
//     system(comando);
// }

// void enviar_lista_arquivos(int sock) {
//     frame_t msg;
//     msg.init_mark = 0b01111110;
//     msg.type = 0b01010;  // Tipo lista
//     msg.seq = 0;
//     msg.size = 0;
//     msg.crc = crc8(&msg);
//     printf("Enviando tipo lista\n");
//     send(sock, &msg, sizeof(frame_t), 0);

//     frame_t ack_msg;

//     if (recv(sock, &ack_msg, sizeof(frame_t), 0) < 0) {
//         printf("ERRO\n");
//         return;
//     }

//     while (ack_msg.type != 0b00000 && ack_msg.init_mark != 126) {
//         if (ack_msg.type == 0b00001) {
//             send(sock, &msg, sizeof(frame_t), 0);
//         }

//         if (recv(sock, &ack_msg, sizeof(frame_t), 0) < 0) {
//             printf("ERRO\n");
//             return;
//         }
//     }

//     printf("recebi ack");

//     frame_t response;

//     if (recv(sock, &response, sizeof(frame_t), 0) < 0) {
//         printf("ERRO\n");
//         return;
//     }

//     unsigned int seq = 0;


//     while (response.type != 0b11110 || response.init_mark != 126) {
//         if (response.init_mark == 126 && response.type == 0b10010) {
//             // if (response.seq == seq) {
//                 if (response.crc == crc8(&response)) {
//                     printf("Disponível: %s\n", response.data);

//                     msg.type = 0b00000;
//                     msg.seq = response.seq;



//                     send(sock, &msg, sizeof(frame_t), 0);

//                 } else {
//                     msg.type = 0b00001;
//                     msg.seq = response.seq;

//                     send(sock, &msg, sizeof(frame_t), 0);
//                 }
//             // }
//         }

//         if (recv(sock, &response, sizeof(frame_t), 0) < 0) {
//             printf("Erro\n");
//             return;
//         }
//     }

//     msg.type = 0b00000;
//     msg.seq = seq;

//     send(sock, &msg, sizeof(frame_t), 0);

//     seq = (seq + 1) % 32;
// }