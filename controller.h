#ifndef CONTROLLER_H
#define CONTROLLER_H

struct frame {
    unsigned int init_mark : 8;
    unsigned int size : 6;
    unsigned int seq : 5;
    unsigned int type : 5;
    char data[63];
    unsigned int crc : 8;
};
typedef struct frame frame_t;

unsigned char crc8(frame_t *frame);
void print_frame (frame_t frame);
void list_files(const char *path);
unsigned char* read_file_to_buffer(const char *caminho, long *file_size);

#endif