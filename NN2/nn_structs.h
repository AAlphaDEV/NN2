#ifndef NN_STRUCTS_H
#define NN_STRUCTS_H

#include <pthread.h>

/****
*** _id=-4 -> req waiting in queue
*** _id=-2 -> req in queue already executed
****/
struct nn_req_data {
    int _id;
    char _req[256];
    char *_argv[32];
    int _argc;
    pthread_t _th;
};

typedef struct nn_req_data nn_req_data_t;

struct nn_req_signal {

};
typedef struct nn_req_signal nn_req_signal_t;

union da_data {
    nn_req_data_t _data;
    nn_req_signal_t _signal;
};
typedef union da_data da_data_t;

#define DATA da_data_t

#endif // NN_STRUCTS_H
