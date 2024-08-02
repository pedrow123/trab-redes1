#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>


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

void print_frame (frame_t frame){
    printf("%d - crc\n", frame.crc);
    printf("%s - data\n", frame.data);
    printf("%d - marcador inicio\n", frame.init_mark);
    printf("%d - sequencia\n", frame.seq);
    printf("%d - tam\n", frame.size);
    printf("%d - tipo\n", frame.type);
}

void list_files(const char *path) {
    struct dirent *entry;
    DIR *dp = opendir(path);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dp))) {
        if (entry->d_type == DT_REG) {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dp);
}

unsigned char* read_file_to_buffer(const char *caminho, long *file_size) {
    FILE *file = fopen(caminho, "rb"); // abrir arquivo em modo binário
    if (!file) {
        perror("Erro ao abrir o arquivo");
        return NULL;
    }

    // Determinar o tamanho do arquivo
    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Alocar memória para armazenar o conteúdo do arquivo
    unsigned char *buffer = (unsigned char *)malloc(*file_size);
    if (!buffer) {
        perror("Erro ao alocar memória");
        fclose(file);
        return NULL;
    }

    // Ler o arquivo inteiro para o buffer
    size_t read_size = fread(buffer, 1, *file_size, file);
    if (read_size != *file_size) {
        perror("Erro ao ler o arquivo");
        free(buffer);
        fclose(file);
        return NULL;
    }

    // Fechar o arquivo
    fclose(file);

    return buffer;
}