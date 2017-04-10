#ifndef DYN_ARRAY_H
#define DYN_ARRAY_H

#include <stdio.h>
#include <stdlib.h>

#include "nn_structs.h"

struct dyn_array {
    DATA *_array;
    size_t length;
    size_t capacity;

    size_t item_sz;
};
typedef struct dyn_array dyn_array_t;

int init_array(dyn_array_t *dyn, size_t initial_sz, size_t item_sz);

int push_item(dyn_array_t *dyn, DATA item);
int push_item_data(dyn_array_t *dyn, nn_req_data_t data);
int push_item_signal(dyn_array_t *dyn, nn_req_signal_t sig);

int get_item(dyn_array_t *dyn, int index, DATA *data);
int get_item_data(dyn_array_t *dyn, int index, nn_req_data_t *data);

int free_array(dyn_array_t *dyn);
int pop_item(dyn_array_t *dyn);
int pop_items(dyn_array_t *dyn, size_t nb);

#endif // DYN_ARRAY_H
