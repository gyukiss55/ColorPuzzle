#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

INT JPEGTest()
{
    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    UINT  num;        // number of image decoders
    UINT  size;       // size, in bytes, of the image decoder array

    ImageCodecInfo* pImageCodecInfo;

    // How many decoders are there?
    // How big (in bytes) is the array of all ImageCodecInfo objects?
    GetImageDecodersSize(&num, &size);

    // Create a buffer large enough to hold the array of ImageCodecInfo
    // objects that will be returned by GetImageDecoders.
    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));

    // GetImageDecoders creates an array of ImageCodecInfo objects
    // and copies that array into a previously allocated buffer. 
    // The third argument, imageCodecInfo, is a pointer to that buffer. 
    GetImageDecoders(num, size, pImageCodecInfo);

    // Display the graphics file format (MimeType)
    // for each ImageCodecInfo object.
    for (UINT j = 0; j < num; ++j)
    {
        wprintf(L"%s\n", pImageCodecInfo[j].MimeType);
    }

    free(pImageCodecInfo);
    GdiplusShutdown(gdiplusToken);
    return 0;
}