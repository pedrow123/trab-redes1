#include <stdio.h>
#include <stdlib.h>

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