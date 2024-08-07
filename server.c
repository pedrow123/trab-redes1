

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "controller.h"
#include "rawsocket.h"

#define WINDOW_SIZE 5
#define MAX_RETRIES 10
#define MAX_PAYLOAD_SIZE 63

int main() {
    int sock = ConexaoRawSocket("lo");
    frame_t msg;
    printf("Servidor iniciado, aguardando mensagens...\n");

    FILE *output_file = fopen("output.txt", "w+");
    if (!output_file) {
        perror("Erro ao criar o arquivo de saída");
        return 1;
    }

    while (1) {
        if (recv(sock, &msg, sizeof(frame_t), 0) < 0) {
            return 1;
        }
        if (msg.type == 0b01010 && msg.init_mark == 126) {
            list_files("videos", sock);
            break;
        }
    }

    char arquivo[63];

    // O servidor deve aguardar a mensagem do cliente
    while (1) {
        if (recv(sock, &msg, sizeof(frame_t), 0) < 0) {
            perror("Erro ao receber mensagem");
            break;
        }

        if (msg.init_mark == 126 &&
            msg.type == 0b01011) {  
            printf("Recebi nome do vídeo: %s\n", msg.data);
            strncpy(arquivo, msg.data, strlen(msg.data));
            break;  
        }
    }

    char caminho[1024];
    printf("%s\n", msg.data);
    snprintf(caminho, sizeof(caminho), "%s/%s", "videos", msg.data);

    long file_size;
    unsigned char *buffer = read_file_to_buffer(caminho, &file_size);
    if (!buffer) {
        // close(sock);
        return 1;
    }

    printf("Arquivo lido com sucesso. Tamanho: %ld bytes\n", file_size);

    msg.init_mark = 0b01111110;
    msg.seq = 0;  // Primeiro pacote
    msg.type = 0b10010;

    int retries = 0;

    struct timeval tv;
    tv.tv_sec = 2;  // Timeout de 2 segundos
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    frame_t ack_msg;

    size_t offset = 0;

    while (offset < file_size) {
        size_t bytes_to_send = (file_size - offset > MAX_PAYLOAD_SIZE)
                                   ? MAX_PAYLOAD_SIZE
                                   : (file_size - offset);
        unsigned char *escaped_data;
        size_t escaped_size;

        escape_data(buffer + offset, bytes_to_send, &escaped_data,
                    &escaped_size);

        memcpy(msg.data, escaped_data, escaped_size);

        if (escaped_size < MAX_PAYLOAD_SIZE) {
            memset(msg.data + escaped_size, '\0',
                   MAX_PAYLOAD_SIZE - escaped_size);
        }

        msg.size = escaped_size;
        msg.crc = crc8(&msg);

        send(sock, &msg, sizeof(frame_t), 0);
        printf("Enviando seq %d: %.*s\n", msg.seq, (int)escaped_size, msg.data);
        log_frame("server_log.txt", &msg, "Enviado");

        retries = 0;
        while (1) {
            if (retries > MAX_RETRIES)
                break;
            
            if (recv(sock, &ack_msg, sizeof(frame_t), 0) > 0) {
                if (ack_msg.init_mark == 126) {
                    if (ack_msg.type == 0b00000 && ack_msg.seq == msg.seq) {
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
                // retries++;
                send(sock, &msg, sizeof(frame_t), 0);
            }
        }

        if (retries == MAX_RETRIES) {
            printf("\nFalha após %d tentativas para seq %d\n", MAX_RETRIES,
                   msg.seq);
            break;
        }


        offset += bytes_to_send;
        msg.seq++;
    }


    // Envia um pacote final indicando o fim da transmissão
    msg.init_mark = 126;
    msg.type = 0b11110;
    msg.size = 0;
    send(sock, &msg, sizeof(frame_t), 0);

    free(buffer);

    close(sock);
    return 0;
}