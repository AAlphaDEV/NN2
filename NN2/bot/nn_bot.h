#ifndef NN_BOT_H
#define NN_BOT_H

#include <pthread.h>

#include "utils.h"
#include "alpha_https.h"
#include "nn_requests.h"
#include "dyn_array.h"
#include "nn_structs.h"

#define MAX_REQ_THREADS 5

/****** GLOBAL VARIABLES ******/

pthread_mutex_t mx_req_threads;
nn_req_data_t req_threads[MAX_REQ_THREADS];

pthread_mutex_t mx_req_count;
unsigned int req_count;

pthread_mutex_t mx_bot_id;
int bot_id;

pthread_mutex_t mx_req_queue;
dyn_array_t req_queue;

/******************************/

unsigned int get_req_count();
void inc_req_count();
void dec_req_count();

int check_signal_queue();
void check_queue();

void bot_startup(char *host, unsigned long request_time);
void bot_cleanup(char *host);
int analyse_recv(char *recv_buf, size_t recv_sz);
void parse_recv(char *req, char **out_params, int *params_count);

void start_request(char *req, char **params, int params_count);
void *request_thread_func(void *arg);

void bot_update(char *host);

void start_log();
void close_log();

#endif // NN_BOT_H
