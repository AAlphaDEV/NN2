#ifndef NN_STRUCTS_H
#define NN_STRUCTS_H

#include <pthread.h>

/****
*** _id=-4 -> req waiting in queue
*** _id=-2 -> req in queue already executed
****/
struct nn_req_data {
    int _id;
    char _req[128];
    char *_argv[32];
    int _argc;
    pthread_t _th;
};

typedef struct nn_req_data nn_req_data_t;

#define RSIG_FROM_REQUEST 1
#define RSIG_FROM_MAIN 2

#define RSIG_TO_REQUEST 3 //NOT IMPLEMENTED
#define RSIG_TO_ALL_REQUESTS 4 //NOT IMPLEMENTED
#define RSIG_TO_MAIN 5 //NOT IMPLEMENTED

#define RSIG_SIG_EXIT 6
#define RSIG_SIG_UPDATE 7

/****
*** _id -> -1 : already received
*** _req -> request name (sender)
*** _from -> sender code
*** _to -> receiver code
*** _sig -> signal code
*** _content -> signal content
*** _free -> 1 if free is needed (on _content)
****/
struct nn_req_signal {
    char _req[128];
    unsigned int _from;
    unsigned int _to;
    unsigned int _sig;
    void *content;
    int _free;
};
typedef struct nn_req_signal nn_req_signal_t;

union da_data {
    nn_req_data_t _data;
    nn_req_signal_t _signal;
};
typedef union da_data da_data_t;

#define DATA da_data_t

#endif // NN_STRUCTS_H
