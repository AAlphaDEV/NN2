#include "alpha_https.h"

void init_ssl()
{
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();
}

int new_https(const char *host, SSL_CTX *ssl_ctx, https_conn_t *https)
{
    BIO *bio;
    SSL *ssl;

    bio = BIO_new_ssl_connect(ssl_ctx);
    if(bio == NULL)
    {
        return -1;
    }

    BIO_get_ssl(bio, &ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    BIO_set_conn_hostname(bio, host);
    if(BIO_do_connect(bio) <= 0) //error
    {
        return -1;
    }

     https->bio = bio;
     https->ssl = ssl;
     https->need_reset = 0;

     return 1;
}

int https_send(https_conn_t *https, const char *buf, int len)
{
    return BIO_write(https->bio, buf, len);
}

int https_read(https_conn_t *https, char *buf, int len)
{
    return BIO_read(https->bio, (void *) buf, len);
}

void https_close(https_conn_t *https)
{
    BIO_free_all(https->bio);
    //SSL_free(https->ssl);
}

int https_request(https_conn_t *https, const char *request, int req_len, char *response, int res_len)
{
    if(https->need_reset)
    {
        BIO_reset(https->bio);
    }

    if(https_send(https, request, req_len) <= 0)
    {
        return -1;
    }

    Sleep(50);

    strcpy(response, "\0");

    int total_sz = 0;

    char buf[4096];
    int r;
    for(;;)
    {
        r = https_read(https, buf, 4096);
        if(r <= 0)
            break;
        if((strlen(response) + r) >= res_len)
        {
            https->need_reset = 1;
            return -2;
        }
        buf[r] = 0;
        total_sz += r;
        strcat(response, buf);
    }

    https->need_reset = 1;

    return total_sz;
}
