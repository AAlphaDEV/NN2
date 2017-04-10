#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

#include <windows.h>
#include <winsock2.h>

#include <curl/curl.h>

void fatalOSSL(char *err_msg, unsigned long errcode);
void fatalWS(char *err_msg, int errcode);
void fatalCURL(char *err_msg, int errcode);
void fatal(char *err_msg);

unsigned long long current_time_millis();
int startwith(char *scmp, char *str);
int occurences(char *str, const char *s);
char *split(char *str, const char c);
char *splits(char *str, const char *sp);

#endif // UTILS_H
