#include "nn_requests.h"

void init_requests()
{
    requests[0].request_func = nn_request_stop;
    strcpy(requests[0].req_name, "stop");

    requests[1].request_func = nn_request_test;
    strcpy(requests[1].req_name, "test");
}

/******
*** -2 -> stop signal
*** -1 -> no request
***  0 -> work fine
******/
int do_request(char *req, char *argv[], int argc)
{
    int r = -1;

    requests_lock();
    int i;
    for(i = 0; i<REQUESTS_COUNT; i++)
    {
        if(strcmp(req, requests[i].req_name) == 0)
        {
            requests_unlock();
            r = requests[i].request_func(argc, argv);
        }
    }
    if(r == -1)
        requests_unlock();

    return r;
}

void requests_lock()
{
    pthread_mutex_lock(&mx_requests);
}

void requests_unlock()
{
    pthread_mutex_unlock(&mx_requests);
}

void req_printf(char *req_name, char *s)
{
    printf("\t[%s] %s\n", req_name, s);
}

int nn_request_stop(int argc, char **argv)
{
    printf("Stop request recevied.\n");
    printf("Stopping bot...\n");
    return -2;
}

int nn_request_test(int argc, char **argv)
{
    char buffer[256];
    char buffer2[128];

    strcpy(buffer, "test request (params : ");
    int i;
    for(i = 0; i<argc; i++)
    {
        sprintf(buffer2, "(%d) \"%s\", ", i, argv[i]);
        strcat(buffer, buffer2);
    }
    strcat(buffer, ")");

    req_printf("test", buffer);
    return 0;
}
