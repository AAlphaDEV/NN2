#ifndef ALPHA_HTTPS_H
#define ALPHA_HTTPS_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>

#define https_conn_t struct https_conn

struct https_conn {
    BIO *bio;
    SSL *ssl;
    int need_reset;
};

void init_ssl();
int new_https(const char *host, SSL_CTX *ssl_ctx, https_conn_t *https);
int https_send(https_conn_t *https, const char *buf, int len);
int https_read(https_conn_t *https, char *buf, int len);
void https_close(https_conn_t *https);

int https_request(https_conn_t *https, const char *request, int req_len, char *response, int res_len);

#endif // ALPHA_HTTPS_H
