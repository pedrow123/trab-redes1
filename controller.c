#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>


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

void log_message(const char* filename, const char* format, ...) {
    FILE* log_file = fopen(filename, "a");
    if (!log_file) {
        perror("Erro ao abrir o arquivo de log");
        return;
    }

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    fclose(log_file);
}

void log_frame(const char* filename, const frame_t* frame, const char* direction) {
    log_message(filename, "%s Frame - init_mark: 0x%x, type: 0x%x, size: %d, seq: %d, crc: 0x%x, data: %.*s\n",
                direction, frame->init_mark, frame->type, frame->size, frame->seq, frame->crc, frame->size, frame->data);
}

void aguarda_confirmacao(int sock, frame_t frame) {
    frame_t response;

    if (recv(sock, &response, sizeof(frame_t), 0) < 0) {
        printf("Erro\n");
        return;
    }

    while (response.init_mark != 126 || response.seq != frame.seq || response.type != 0b00000) {
        
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

void escape_data(const unsigned char *input, size_t input_len, unsigned char **output, size_t *output_len) {
    size_t estimated_len = input_len * 2;
    unsigned char *escaped_data = (unsigned char *)malloc(estimated_len);
    size_t j = 0;

    for (size_t i = 0; i < input_len; i++) {
        if (input[i] == 0x88 && i + 1 < input_len && input[i + 1] == 0x81) {
            escaped_data[j++] = 0x88;
            escaped_data[j++] = 0x81;
            escaped_data[j++] = 0xFF;
            i++;
        } else {
            escaped_data[j++] = input[i];
        }
    }

    *output = (unsigned char *)realloc(escaped_data, j);
    *output_len = j;
}

void deescape_data(const unsigned char *input, size_t input_len, unsigned char **output, size_t *output_len) {
    unsigned char *deescaped_data = (unsigned char *)malloc(input_len);
    size_t j = 0;

    for (size_t i = 0; i < input_len; i++) {
        if (input[i] == 0x88 && i + 2 < input_len && input[i + 1] == 0x81 && input[i + 2] == 0xFF) {
            deescaped_data[j++] = 0x88;
            deescaped_data[j++] = 0x81;
            i += 2;
        } else {
            deescaped_data[j++] = input[i];
        }
    }

    *output = (unsigned char *)realloc(deescaped_data, j);
    *output_len = j;
}
