#include "nn_requests.h"

void init_requests()
{
    requests[0].request_func = nn_request_stop;
    strcpy(requests[0].req_name, "stop");

    requests[1].request_func = nn_request_test;
    strcpy(requests[1].req_name, "test");

    requests[2].request_func = nn_request_update;
    strcpy(requests[2].req_name, "update");

    requests[3].request_func = nn_request_scroff;
    strcpy(requests[3].req_name, "scroff");

    requests[4].request_func = nn_request_scrcap;
    strcpy(requests[4].req_name, "scrcap");

    init_array(&signal_queue, 8, sizeof(DATA));
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

void push_sig_queue(nn_req_signal_t signal)
{
    pthread_mutex_lock(&mx_signal_queue);
    push_item_signal(&signal_queue, signal);
    pthread_mutex_unlock(&mx_signal_queue);
}

void req_printf(char *req_name, char *s)
{
    printf("\t[%s] %s\n", req_name, s);
}

int nn_request_stop(int argc, char **argv)
{
    req_printf("stop", "Stop request received.");
    req_printf("stop", "Sending stop signal...");

    nn_req_signal_t signal;
    signal._free = 0;
    signal._from = RSIG_FROM_REQUEST;
    signal._to = RSIG_TO_MAIN;
    signal._sig = RSIG_SIG_EXIT;

    pthread_mutex_lock(&mx_signal_queue);
    push_item_signal(&signal_queue, signal);
    pthread_mutex_unlock(&mx_signal_queue);

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

int nn_request_update(int argc, char **argv)
{
    req_printf("update", "Update request received.");
    req_printf("update", "Sending update signal...");

    nn_req_signal_t signal;
    signal._free = 0;
    signal._from = RSIG_FROM_REQUEST;
    signal._to = RSIG_TO_MAIN;
    signal._sig = RSIG_SIG_UPDATE;

    pthread_mutex_lock(&mx_signal_queue);
    push_item_signal(&signal_queue, signal);
    pthread_mutex_unlock(&mx_signal_queue);

    return -2;
}

int nn_request_scroff(int argc, char **argv) //Usage : srcoff [on/off] [time]
{
    int off;
    int time;
    char buffer[128];

    if(argc < 1)
    {
        off = 1;
        time = 0;
    } else if(argc == 1)
    {
        off = (strcmp(argv[0], "off") == 0) ? 1 : 0;
        time = 0;
    } else if(argc >= 2)
    {
        off = (strcmp(argv[0], "off") == 0) ? 1 : 0;
        time = atoi(argv[1]);
    }

    sprintf(buffer, "Screen manager -> parameters : off=%d - time=%d", off, time);
    req_printf("scroff", buffer);

    if(off)
    {
        SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)2);
        if(time != 0)
        {
            Sleep(time);
            SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)-1);
        }
    } else
    {
        SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)-1);
    }

    return 0;
}
size_t read_callback(char *bufptr, size_t size, size_t nitems, void *userp)
{
    FILE *file = *((FILE **) userp);
    return fread(bufptr, size, nitems, file);
}

int nn_request_scrcap(int argc, char **argv)
{
    char buffer[512];
    CURL *curl;
    char file_to_upload[260];
    FILE *file;
    curl_off_t file_sz;

    int w, h;
    GetDesktopResolution(&w, &h);
    ScreenCapture(0, 0, w, h, "capture.png");

    strcpy(file_to_upload, "capture.png");

    file = fopen(file_to_upload, "rb");
    if(file == NULL)
    {
        sprintf(buffer, "file not found (aborting)");
        req_printf("scrcap", buffer);
        return 2;
    }

    fseek(file, 0, SEEK_END);
    file_sz = ftell(file);
    fseek(file, 0, SEEK_SET);

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl == NULL)
    {
        sprintf(buffer, "failed to initialize curl. (aborting)");
        req_printf("scrcap", buffer);

        fclose(file);
        curl_global_cleanup();
        return 1;
    }

    strcpy(buffer, "https://nonameufr.alwaysdata.net/file_upload.php");
    curl_easy_setopt(curl, CURLOPT_URL, buffer);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &file);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, file_sz);

    curl_easy_setopt(curl, CURLOPT_USERAGENT, "NN2 Windows");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

    CURLcode res;
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
        sprintf(buffer, "failed to upload file.");
        req_printf("scrcap", buffer);

        fclose(file);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 3;
    }

    fclose(file);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    sprintf(buffer, "File successfully uploaded.");
    req_printf("scrcap", buffer);

    remove("capture.png");

    return 0;
}
