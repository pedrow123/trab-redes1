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
void list_files(const char *path, int sock);
unsigned char* read_file_to_buffer(const char *caminho, long *file_size);
void log_message(const char* filename, const char* format, ...);
void log_frame(const char* filename, const frame_t* frame, const char* direction);
void escape_data(const unsigned char *input, size_t input_len, unsigned char **output, size_t *output_len) ;
void deescape_data(const unsigned char *input, size_t input_len, unsigned char **output, size_t *output_len);
void aguarda_confirmacao(int sock, frame_t frame) ;

#endif