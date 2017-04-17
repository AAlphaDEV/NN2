#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <shlobj.h>

#include "unzip.h"

#define dir_delimter '/'
#define MAX_FILENAME 512
#define READ_SIZE 8192

char *parent_frompath(char *str)
{
    if(strchr(str, '\\') == NULL && strchr(str, '/') == NULL)
        return str;
    int index = -1;
    size_t str_sz = strlen(str);
    int i;
    for(i = 0; i<str_sz; i++)
    {
        if(str[i] == '\\' || str[i] == '/')
            index = i;
    }
    if(index == -1)
    {
        str[0] = '.';
        str[1] = 0;
        return str;
    }
    if(index+1 >= str_sz)
        return str;
    str[index] = 0;
    return str;
}

void temp()
{
    char buffer[260];
    sprintf(buffer, "\"%s\\%s\"", "D:\\H\\C\\NN2\\workspace\\NN2\\bin\\Debug", "NN2.exe");

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // start the program up
    CreateProcess(NULL,   // the path
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
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return ;
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    FILE *file = (FILE *) userdata;
    return fwrite(ptr, size, nmemb, file);
}

int get_file(char *src)
{
    FILE *file;
    CURL *curl;
    char tmpfile[260];
    char buffer2[128];
    char buffer[512];

    sprintf(tmpfile, "%s\\app.zip", src);

    file = fopen(tmpfile, "wb");

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl == NULL)
    {
        sprintf(buffer, "[!!] Update error : failed to initialize curl.d (aborting)\n");
        fprintf(stderr, buffer);

        fclose(file);
        curl_global_cleanup();

        exit(-1);
    }

    strcpy(buffer2, "nonameufr.alwaysdata.net");
    sprintf(buffer, "https://%s/install/install.php", buffer2);

    //printf("URL : %s\n", buffer);

    curl_easy_setopt(curl, CURLOPT_URL, buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "NN2Install Windows");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

    CURLcode res;
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
        sprintf(buffer, "[!!] Update error : %s (aborting)\n", curl_easy_strerror(res));
        fprintf(stderr, buffer);

        fclose(file);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        exit(-1);
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    fclose(file);


    return 0;
}

int uncompress_file(char *src, char *output_folder)
{
    char buffer[260];
    char zippath[260];

    #ifndef UPDATER
    strcat(output_folder, "\\winsec\\");
    mkdir(output_folder);
    #endif // UPDATER

    //printf("output folder : %s\n", output_folder);

    sprintf(zippath, "%s\\app.zip", src);
    unzFile *zipfile = unzOpen(zippath);
    if(zipfile == NULL)
    {
        printf("%s: not found\n", "app.zip");
        return -1;
    }

    unz_global_info global_info;
    if(unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK)
    {
        printf("could not read file global info\n");
        unzClose(zipfile);
        return -1;
    }

    //Buffer to hold data read from the zip file.
    char read_buffer[READ_SIZE];

    //Loop to extract all files
    uLong i;
    for (i = 0; i<global_info.number_entry; i++)
    {
        //Get info about current file.
        unz_file_info file_info;
        char filename[MAX_FILENAME];
        if(unzGetCurrentFileInfo(
            zipfile,
            &file_info,
            filename,
            MAX_FILENAME,
            NULL, 0, NULL, 0) != UNZ_OK)
        {
            printf("could not read file info\n");
            unzClose(zipfile);
            remove(zippath);
            return -1;
        }

        // Check if this entry is a directory or file.
        const size_t filename_length = strlen(filename);
        if(filename[filename_length-1] == dir_delimter)
        {
            // Entry is a directory, so create it.
            //printf("dir:%s\n", filename);
            sprintf(buffer, "%s%s", output_folder, filename);
            mkdir(buffer);
        }
        else
        {
            // Entry is a file, so extract it.
            //printf("file:%s\n", filename);
            if(unzOpenCurrentFile(zipfile) != UNZ_OK)
            {
                printf("could not open file\n");
                unzClose(zipfile);
                remove(zippath);
                return -1;
            }

            // Open a file to write out the data.
            sprintf(buffer, "%s%s", output_folder, filename);
            FILE *out = fopen(buffer, "wb");
            printf("file=%s\n", buffer);
            if(out == NULL)
            {
                printf("could not open destination file : %s\n", buffer);
                unzCloseCurrentFile(zipfile);

                // Go the the next entry listed in the zip file.
                if((i+1) < global_info.number_entry)
                {
                    if(unzGoToNextFile(zipfile) != UNZ_OK)
                    {
                        printf("cound not read next file\n");
                        unzClose(zipfile);
                        remove(zippath);
                        return -1;
                    }
                }
                continue;
            }

            int error = UNZ_OK;
            do
            {
                error = unzReadCurrentFile(zipfile, read_buffer, READ_SIZE);
                if(error < 0)
                {
                    printf("error %d\n", error);
                    unzCloseCurrentFile(zipfile);
                    unzClose(zipfile);
                    return -1;
                }

                // Write data to file.
                if(error > 0)
                {
                    fwrite(read_buffer, error, 1, out); // You should check return of fwrite...
                }
            } while (error > 0);

            fclose(out);
        }

        unzCloseCurrentFile(zipfile);

        // Go the the next entry listed in the zip file.
        if((i+1) < global_info.number_entry)
        {
            if(unzGoToNextFile(zipfile) != UNZ_OK)
            {
                printf("cound not read next file\n");
                unzClose(zipfile);
                remove(zippath);
                return -1;
            }
        }
    }

    unzClose(zipfile);
    return 0;
}

int main(int argc, char *argv[])
{
    CURL *curl;
    char buffer[260];
    char src[MAX_PATH];
    FILE *file;
    int ret;

    /*** Get current folder ***/
    if(GetModuleFileName(NULL, buffer, 512) == 512)
    {
        printf("[!!] Error while getting module filename.\n");
        strcpy(src, ".");
    } else
    {
        strcpy(src, parent_frompath(buffer));
    }

    printf("Getting files...\n");
    ret = get_file(src);
    if(ret != 0)
    {
        printf("Failed to get files\n");
        return -1;
    }

    char output_folder[MAX_PATH];

    #ifdef UPDATER
    sprintf(output_folder, "%s\\", src);
    #else
    if(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, output_folder) != S_OK)
    {
        printf("[-] Failed to retrieve local APPDATA folder, trying common APPDATA\n");
        if(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, output_folder) != S_OK)
        {
            printf("[-] Failed to retrieve common APPDATA folder, aborting...\n");
            return -1;
        }
    }
    #endif // UPDATER

    printf("Uncompressing files...\n");
    printf("src=%s\noutput_foler=%s\n", src, output_folder);

    ret = uncompress_file(src, output_folder);
    if(ret != 0)
    {
        printf("Failed to uncompress file\n");
        return -1;
    }

    printf("Removing temp files...\n");
    sprintf(buffer, "%s\\app.zip", src);
    remove(buffer);

    printf("Launching program...\n");
    sprintf(buffer, "\"%s\\%s\"", output_folder, "winsec.exe");

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // start the program up
    CreateProcess(NULL,   // the path
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
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    printf("Done. Exiting installer...\n");
    return 0;
}
