#include "screenshot.h"

// Helper function to retrieve current position of file pointer:
int GetFilePointer(HANDLE FileHandle)
{
	return SetFilePointer(FileHandle, 0, 0, FILE_CURRENT);
}

int SaveBMPFile(char *filename, HBITMAP bitmap, HDC bitmapDC, int width, int height)
{
    HBITMAP OffscrBmp = NULL; // bitmap that is converted to a DIB
    HDC OffscrDC = NULL;      // offscreen DC that we can select OffscrBmp into
    LPBITMAPINFO lpbi = NULL; // bitmap format info; used by GetDIBits
    LPVOID lpvBits = NULL;    // pointer to bitmap bits array
    HANDLE BmpFile = INVALID_HANDLE_VALUE;    // destination .bmp file
    BITMAPFILEHEADER bmfh;  // .bmp file header

    // We need an HBITMAP to convert it to a DIB:
    if ((OffscrBmp = CreateCompatibleBitmap(bitmapDC, width, height)) == NULL)
        return 0;

    // The bitmap is empty, so let's copy the contents of the surface to it.
    // For that we need to select it into a device context. We create one.
    if ((OffscrDC = CreateCompatibleDC(bitmapDC)) == NULL)
		return 0;

    // Select OffscrBmp into OffscrDC:
    HBITMAP OldBmp = (HBITMAP)SelectObject(OffscrDC, OffscrBmp);

    // Now we can copy the contents of the surface to the offscreen bitmap:
    BitBlt(OffscrDC, 0, 0, width, height, bitmapDC, 0, 0, SRCCOPY);

    // GetDIBits requires format info about the bitmap. We can have GetDIBits
    // fill a structure with that info if we pass a NULL pointer for lpvBits:
    // Reserve memory for bitmap info (BITMAPINFOHEADER + largest possible
    // palette):
    if((lpbi = (LPBITMAPINFO)(malloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)))) == NULL)
		return 0;

    memset(&(lpbi->bmiHeader), 0, sizeof(BITMAPINFOHEADER));
    lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    // Get info but first de-select OffscrBmp because GetDIBits requires it:
    SelectObject(OffscrDC, OldBmp);
    if(!GetDIBits(OffscrDC, OffscrBmp, 0, height, NULL, lpbi, DIB_RGB_COLORS))
        return 0;

    // Reserve memory for bitmap bits:
    if((lpvBits = malloc(lpbi->bmiHeader.biSizeImage)) == NULL)
        return 0;

    // Have GetDIBits convert OffscrBmp to a DIB (device-independent bitmap):
    if (!GetDIBits(OffscrDC, OffscrBmp, 0, height, lpvBits, lpbi, DIB_RGB_COLORS))
		return 0;

    // Create a file to save the DIB to:
    if ((BmpFile = CreateFile(filename,
                              GENERIC_WRITE,
                              0, NULL,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL)) == INVALID_HANDLE_VALUE)

							  return 0;

    DWORD Written;    // number of bytes written by WriteFile

    // Write a file header to the file:
    bmfh.bfType = 19778;        // 'BM'
    // bmfh.bfSize = ???        // we'll write that later
    bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
    // bmfh.bfOffBits = ???     // we'll write that later
    if (!WriteFile(BmpFile, &bmfh, sizeof(bmfh), &Written, NULL))
        return 0;

    if (Written < sizeof(bmfh))
		return 0;

    // Write BITMAPINFOHEADER to the file:
    if (!WriteFile(BmpFile, &lpbi->bmiHeader, sizeof(BITMAPINFOHEADER), &Written, NULL))
		return 0;

    if (Written < sizeof(BITMAPINFOHEADER))
			return 0;

    // Calculate size of palette:
    int PalEntries;
    // 16-bit or 32-bit bitmaps require bit masks:
    if (lpbi->bmiHeader.biCompression == BI_BITFIELDS)
		PalEntries = 3;
    else
        // bitmap is palettized?
        PalEntries = (lpbi->bmiHeader.biBitCount <= 8) ?
            // 2^biBitCount palette entries max.:
            (int)(1 << lpbi->bmiHeader.biBitCount)
        // bitmap is TrueColor -> no palette:
        : 0;
    // If biClrUsed use only biClrUsed palette entries:
    if(lpbi->bmiHeader.biClrUsed)
		PalEntries = lpbi->bmiHeader.biClrUsed;

    // Write palette to the file:
    if(PalEntries)
    {
        if (!WriteFile(BmpFile, &lpbi->bmiColors, PalEntries * sizeof(RGBQUAD), &Written, NULL))
			return 0;
        if (Written < PalEntries * sizeof(RGBQUAD))
			return 0;
	}

    // The current position in the file (at the beginning of the bitmap bits)
    // will be saved to the BITMAPFILEHEADER:
    bmfh.bfOffBits = GetFilePointer(BmpFile);

    // Write bitmap bits to the file:
    if (!WriteFile(BmpFile, lpvBits, lpbi->bmiHeader.biSizeImage, &Written, NULL))
		return 0;

    if (Written < lpbi->bmiHeader.biSizeImage)
		return 0;

    // The current pos. in the file is the final file size and will be saved:
    bmfh.bfSize = GetFilePointer(BmpFile);

    // We have all the info for the file header. Save the updated version:
    SetFilePointer(BmpFile, 0, 0, FILE_BEGIN);
    if (!WriteFile(BmpFile, &bmfh, sizeof(bmfh), &Written, NULL))
        return 0;

    if (Written < sizeof(bmfh))
		return 0;

    free(lpvBits);
    free(lpbi);

    CloseHandle(BmpFile);

    return 1;
}

void GetDesktopResolution(int *horizontal, int *vertical)
{
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();

    GetWindowRect(hDesktop, &desktop);
    *horizontal = desktop.right;
    *vertical = desktop.bottom;
}

int ScreenCapture(int x, int y, int width, int height, char *filename)
{
    char bmpfilename[256];

    HDC hDc = CreateCompatibleDC(0);
    HBITMAP hBmp = CreateCompatibleBitmap(GetDC(0), width, height);
    SelectObject(hDc, hBmp);

    BitBlt(hDc, 0, 0, width, height, GetDC(0), x, y, SRCCOPY);

    sprintf(bmpfilename, "%s.bmp", filename);
    int ret = SaveBMPFile(bmpfilename, hBmp, hDc, width, height);
    DeleteObject(hBmp);
    if(ret != 1)
    {
        return -1;
    }

    unsigned char *bmpfile;
    size_t bmpfile_sz;

    int r = lodepng_load_file(&bmpfile, &bmpfile_sz, bmpfilename);
    if(r != 0)
    {
        free(bmpfile);
        return r;
    }

    unsigned int w;
    unsigned int h;

    unsigned char* image = NULL;
    ret = decodeBMP(&image, &w, &h, bmpfile, &bmpfile_sz);
    if(image == NULL)
    {
        fprintf(stderr, "[!!] Failed to decode BMP image\n");
        return -1;
    }

    unsigned int error = lodepng_encode32_file(filename, image, w, h);
    if(error)
    {
        printf("[!!] Lodepng error (errcode=%u) : %s\n", error, lodepng_error_text(error));
        return -1;
    }

    remove(bmpfilename);

    free(bmpfile);
    if(image != NULL)
        free(image);

    return ret;
}

int decodeBMP(unsigned char** image, unsigned int* w, unsigned int* h, const unsigned char* bmp, const size_t* bmp_sz)
{
    static const unsigned MINHEADER = 54; //minimum BMP header size

    if(*bmp_sz < MINHEADER)
    {
        return -1;
    }
    if(bmp[0] != 'B' || bmp[1] != 'M')
    {
        return 1;
    }

    unsigned pixeloffset = bmp[10] + 256 * bmp[11];
    *w = bmp[18] + bmp[19] * 256;
    *h = bmp[22] + bmp[23] * 256;

    if(bmp[28] != 24 && bmp[28] != 32)
    {
        return 2; //only 24-bit and 32-bit BMPs are supported.
    }
    unsigned numChannels = bmp[28] / 8;

    unsigned scanlineBytes = (*w) * numChannels;
    if(scanlineBytes % 4 != 0)
    {
        scanlineBytes = (scanlineBytes / 4) * 4 + 4;
    }

    unsigned dataSize = scanlineBytes * (*h);
    if(*bmp_sz < dataSize + pixeloffset)
    {
        return 3; //BMP file too small to contain all pixels
    }

    *image = (unsigned char *) malloc((*w) * (*h) * 4);
    if(*image == NULL)
    {
        return 4; //Failed to allocate memory
    }

    unsigned int y;
    for(y = 0; y < (*h); y++)
    {
        unsigned int x;
        for(x = 0; x < (*w); x++)
        {
            unsigned bmpos = pixeloffset + ((*h) - y - 1) * scanlineBytes + numChannels * x;
            unsigned newpos = 4 * y * (*w) + 4 * x;

            if(numChannels == 3)
            {
              (*image)[newpos + 0] = bmp[bmpos + 2]; //R
              (*image)[newpos + 1] = bmp[bmpos + 1]; //G
              (*image)[newpos + 2] = bmp[bmpos + 0]; //B
              (*image)[newpos + 3] = 255;            //A
            } else
            {
              (*image)[newpos + 0] = bmp[bmpos + 2]; //R
              (*image)[newpos + 1] = bmp[bmpos + 1]; //G
              (*image)[newpos + 2] = bmp[bmpos + 0]; //B
              //(*image)[newpos + 3] = bmp[bmpos + 3]; //A
              (*image)[newpos + 3] = 255; //A

              //printf("RGBA : %d %d %d %d\n", bmp[bmpos + 2], bmp[bmpos + 1], bmp[bmpos + 0], bmp[bmpos + 3]);
            }
        }
    }
    return 0;
}
