#include "nn_bot.h"

void bot_update(char *host)
{
    char exefolder[260];
    char buffer[512];
    char changer[260];

    /*** Stopping bot ***/
    bot_cleanup(host);
    Sleep(100);

    /*** Get current filename ***/
    if(GetModuleFileName(NULL, buffer, 512) == 512)
    {
        printf("[!!] Error while getting module filename.\n");
        strcpy(exefolder, ".");
    } else
    {
        strcpy(exefolder, parent_frompath(buffer));
    }

    /*** STARTING changer.exe ***/
    sprintf(changer, "%s\\changer.exe", exefolder);
    printf("changer=%s\n", changer);
    sprintf(buffer, "\"%s\"", changer);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    printf("command line=%s\n", buffer);
    // start the program up
    int ret = CreateProcess(changer,   // the path
    buffer,        // Command line
    NULL,           // Process handle not inheritable
    NULL,           // Thread handle not inheritable
    FALSE,          // Set handle inheritance to FALSE
    0,              // No creation flags
    NULL,           // Use parent's environment block
    NULL,           // Use parent's starting directory
    &si,            // Pointer to STARTUPINFO structure
    &pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    );
    // Close process and thread handles.

    if(ret == 0)
    {
        fatalWS("while launching changer.exe process", GetLastError());
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    /******************************/

    printf("Done. Exiting bot...\n");

    close_log();

    exit(0);
}
