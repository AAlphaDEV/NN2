#ifndef NN_REQUESTS_H
#define NN_REQUESTS_H

#include <pthread.h>

#include "utils.h"
#include "dyn_array.h"
#include "screenshot.h"

#define REQUESTS_COUNT 5

struct nn_request {
    int(*request_func)(int, char **);
    char req_name[16];
};

typedef struct nn_request nn_request_t;

pthread_mutex_t mx_requests;
nn_request_t requests[REQUESTS_COUNT];

pthread_mutex_t mx_signal_queue;
dyn_array_t signal_queue;

void init_requests();

/****
*** -1 -> no request found
****/
int do_request(char *req, char *argv[], int argc);

void requests_lock();
void requests_unlock();

void push_sig_queue(nn_req_signal_t signal);

/**** Request's util functions ****/
void req_printf(char *req_name, char *s);
/**********************************/

/***** Request's functions *****/
int nn_request_stop(int argc, char **argv);
int nn_request_test(int argc, char **argv);
int nn_request_update(int argc, char **argv);
int nn_request_scroff(int argc, char **argv);
int nn_request_scrcap(int argc, char **argv);
/*******************************/

#endif // NN_REQUESTS_H
