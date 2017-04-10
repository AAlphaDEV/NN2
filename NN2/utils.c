#include "utils.h"

void fatal(char *err_msg)
{
    char error[128];
    strcpy(error, "[!!] Fatal Error ");
    strncat(error, err_msg, 105);

    perror(error);
    exit(-1);
}

void fatalCURL(char *err_msg, int errcode)
{
    char error[512];
    strcpy(error, "[!!] CURL Fatal Error ");
    strncat(error, err_msg, 100);
    strcat(error, " : ");

    strcat(error, curl_easy_strerror(errcode));

    strcat(error, "\n");

    fprintf(stderr, error);
}

void fatalOSSL(char *err_msg, unsigned long errcode)
{
    char error[512];
    strcpy(error, "[!!] OpenSSL Fatal Error ");
    strncat(error, err_msg, 100);
    strcat(error, " : ");

    ERR_print_errors_fp(stderr);
    strcat(error, ERR_error_string(errcode, NULL));

    strcat(error, "\n");

    fprintf(stderr, error);
}

void fatalWS(char *err_msg, int errcode)
{
    char error[128];
    strcpy(error, "[!!] Windows Fatal Error ");
    strncat(error, err_msg, 105);
    strcat(error, " : ");

    char err_id[10];
    sprintf(err_id, "[%d] ", errcode);
    strcat(error, err_id);

    char *s = NULL;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, errcode,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                   (LPSTR)&s, 0, NULL);
    strcat(error, s);
    LocalFree(s);

    fprintf(stderr, "%s", error);
}

unsigned long long current_time_millis()
{
    time_t seconds;

    time(&seconds);
    return (unsigned long long) seconds * 1000;
}

int startwith(char *scmp, char *str)
{
    int sz = strlen(scmp);
    if(strlen(str) < sz)
        return -1;
    char buf = str[sz];
    str[sz] = 0;
    int r = strcmp(scmp, str);
    str[sz] = buf;
    return r;
}

int occurences(char *str, const char *s)
{
    unsigned int i = 0;
    unsigned int j;
    unsigned int s_sz = strlen(s);
    unsigned int str_sz = strlen(str);

    unsigned int occ = 0;

    while(str[i] != '\0') //jejeudi i=0
    {
        for(j = 0; j<=s_sz; j++)
        {
            if(s[j] == '\0')
            {
                occ++;
                i += j;
                break;
            }
            if(i+j > str_sz)
            {
                return occ;
            }

            if(str[i+j] == s[j])
            {
                continue;
            }
            i += j;
            i = (j == 0) ? i+1 : i;
            break;
        }
    }

    return occ;
}

char *split(char *str, const char c)
{
    unsigned int str_sz = strlen(str);
    int i;
    for(i = 0; i<str_sz; i++)
    {
        if(str[i] == c)
        {
            str[i] = 0;
            return str;
        }
    }
    return str;
}

char *splits(char *str, const char *sp)
{
    unsigned int str_sz = strlen(str);
    unsigned int sp_sz = strlen(sp);
    int i;
    for(i = 0; i<str_sz; i++)
    {
        if(str[i] == sp[0])
        {
            int j;
            for(j = 0; j<sp_sz+1; j++)
            {
                if(sp[j] == '\0')
                {
                    str[i] = 0;
                    return str;
                }
                if(i+j >= str_sz)
                    return str;
                if(str[i+j] != sp[j])
                {
                    break;
                }
            }
        }
    }
    return str;
}
