#include "dyn_array.h"

int init_array(dyn_array_t *dyn, size_t initial_sz, size_t item_sz)
{
    dyn->item_sz = item_sz;
    dyn->length = 0;
    dyn->capacity = initial_sz;
    void *_tmp = (void *) malloc(initial_sz * item_sz);
    if(_tmp == NULL)
    {
        fprintf(stderr, "[!!] Error : failed to allocate array's memory.\n");
        return -1;
    }
    dyn->_array = (DATA *) _tmp;
    return 0;
}

int push_item(dyn_array_t *dyn, DATA item)
{
    if(dyn->_array == NULL)
    {
        fprintf(stderr, "[!!] Error : array uninitialized.\n");
        return -1;
    }
    if(dyn->length >= dyn->capacity)
    {
        //printf("reallocation\n");
        dyn->capacity = (dyn->capacity == 0) ? 1 : dyn->capacity*2;
        void *_tmp = realloc((void *) dyn->_array, dyn->capacity * dyn->item_sz);
        if(_tmp == NULL)
        {
            fprintf(stderr, "[!!] Error : failed to reallocate array's memory.\n");
            return -1;
        }
        dyn->_array = _tmp;
    }

    dyn->_array[dyn->length] = item;
    dyn->length++;

    return 0;
}

int remove_item(dyn_array_t *dyn, int index)
{
    return -1;
    if(dyn->_array == NULL || dyn->length == 0)
        return -1;
    void *_tmp = malloc(dyn->capacity * dyn->item_sz);
    if(_tmp == NULL)
    {
        fprintf(stderr, "[!!] Error : failed to reallocate array's memory.\n");
        return -1;
    }

    if(index != 0)
        memmove(_tmp, dyn->_array, index * dyn->item_sz); //copy everything BEFORE the index

    printf("sizeof : %d\n", dyn->item_sz);
    printf("size : %x\n", (int) (dyn->_array + (index+1)));
    printf("other : %d\n", (dyn->length - index - 1) * dyn->item_sz);
    if(index != (dyn->length - 1))
        memmove((void *) (_tmp + index), (void *) (dyn->_array + (index+1)), (dyn->length - index - 1) * dyn->item_sz); //copy everything AFTER the index

    free(dyn->_array);
    dyn->_array = _tmp;
    dyn->length--;
    return 0;
}

int push_item_data(dyn_array_t *dyn, nn_req_data_t data)
{
    da_data_t item;
    item._data = data;
    return push_item(dyn, item);
}

int push_item_signal(dyn_array_t *dyn, nn_req_signal_t sig)
{
    da_data_t item;
    item._signal = sig;
    return push_item(dyn, item);
}

int pop_item(dyn_array_t *dyn)
{
    dyn->length--;

    return 0;
}

int pop_items(dyn_array_t *dyn, size_t nb)
{
    int r = 0;
    int i;
    for(i = 0; i<nb; i++)
    {
        if(pop_item(dyn) != 0)
        {
            r = -1;
        }
    }
    return r;
}

int get_item(dyn_array_t *dyn, int index, DATA *data)
{
    if(index >= dyn->length || index < 0)
    {
        return -1;
    }
    *data = dyn->_array[index];
    return 0;
}

int get_item_data(dyn_array_t *dyn, int index, nn_req_data_t *data)
{
    da_data_t un;
    int r = get_item(dyn, index, &un);
    *data = un._data;
    return r;
}

int get_item_signal(dyn_array_t *dyn, int index, nn_req_signal_t *data)
{
    da_data_t un;
    int r = get_item(dyn, index, &un);
    *data = un._signal;
    return r;
}

int free_array(dyn_array_t *dyn)
{
    free((void *) dyn->_array);
    dyn->length = dyn->capacity = 0;
    dyn->_array = NULL;

    return 0;
}
