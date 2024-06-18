#ifndef CONTROLLER_H
#define CONTROLLER_H

struct frame {
    unsigned int init_mark : 8;
    unsigned int size : 6;
    unsigned int seq : 5;
    unsigned int type : 5;
    char* data;
    unsigned int crc : 8;
};
typedef struct frame frame_t;

#endif