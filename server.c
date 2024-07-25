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
//     frame_t msg;
//     printf("Recebendo\n");
//     int i = 0;

//     while (1) {
//         if (recv(sock, &msg, sizeof(frame_t), 0) < 0) {
//             return 1;
//         }

//         if (msg.init_mark == 126 && msg.type == 0b10010) {
//             unsigned char received_crc = msg.crc;
//             unsigned char calculated_crc = 146;
//             // unsigned char calculated_crc = crc8(&msg);

//             frame_t received_msg;
//             received_msg.init_mark = 0b01111110;
//             received_msg.size = 0;
//             received_msg.seq = msg.seq;
//             received_msg.crc = crc8(&msg);

//             if (received_crc == calculated_crc) {
//                 printf("Recebi: %s\n", msg.data);
//                 received_msg.type = 0b00000;  // ACK
//                 // print_frame(received_msg);
//                 if (send(sock, &received_msg, sizeof(frame_t), 0) < 0)
//                     perror("Erro");

//                 break;

//             } else {
//                 // printf("ENTREI NO NACK UHUUU\n");
//                 received_msg.type = 0b00001;  // nACK
//                 if (send(sock, &received_msg, sizeof(frame_t), 0) < 0)
//                     perror("Erro");
//             }
//         }

//     }
//     close(sock);
//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

#include "controller.h"
#include "rawsocket.h"

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

int main() {
    int sock = ConexaoRawSocket("lo");
    frame_t msg;
    printf("Recebendo\n");

    while (1) {
        if (recv(sock, &msg, sizeof(frame_t), 0) < 0) {
            return 1;
        }

        if (msg.init_mark == 126 && msg.type == 0b10010) {
            unsigned char received_crc = msg.crc;
            unsigned char calculated_crc = crc8(&msg);

            frame_t received_msg;
            received_msg.init_mark = 0b01111110;
            received_msg.size = 0;
            received_msg.seq = msg.seq;
            received_msg.crc = crc8(&msg);

            if (received_crc == calculated_crc) {
                printf("Recebi: %s\n", msg.data);
                received_msg.type = 0b00000;  // ACK
                if (send(sock, &received_msg, sizeof(frame_t), 0) < 0)
                    perror("Erro");

                break;
            } else {
                received_msg.type = 0b00001;  // nACK
                if (send(sock, &received_msg, sizeof(frame_t), 0) < 0)
                    perror("Erro");
            }
        }
    }

    close(sock);
    return 0;
}

