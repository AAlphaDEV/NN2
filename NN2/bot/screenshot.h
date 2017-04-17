#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "lodepng.h"

int SaveBMPFile(char *filename, HBITMAP bitmap, HDC bitmapDC, int width, int height);
int ScreenCapture(int x, int y, int width, int height, char *filename);
void GetDesktopResolution(int *horizontal, int *vertical);

int decodeBMP(unsigned char** image, unsigned int* w, unsigned int* h, const unsigned char* bmp, const size_t* bmp_sz);

#endif
