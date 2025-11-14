// ====================================================================
// snapshot utility
//
// January, 1998 jerry@mail.csh.rit.edu
//   http://www.csh.rit.edu/~jerry
// ====================================================================
//
// snap.c
//

#include "../INCLUDE/allegro.h"
#include <stdio.h>
#include "../INCLUDE/snap.h"    // hey, we need these!
#ifdef SNAP_JPEG
#include <../INCLUDE/jpeg.h>
#endif
#include <windows.h>    // for GetFileAttributes
#include <direct.h>     // for _mkdir
#include "../INCLUDE/GUIPAL.H"

extern MYBITMAP* screen;          // Main display bitmap

// Add this function declaration if not already present
RGB* get_current_palette(void);

void snap(MYBITMAP* bmp, RGB* pal)
{
    FILE* fp;
    char filename[80];
    int counter = 0;
    int done = 0;

    // Check if directory exists using Windows API
    DWORD attrib = GetFileAttributes(SNAPDIR);
    if (attrib == INVALID_FILE_ATTRIBUTES || !(attrib & FILE_ATTRIBUTE_DIRECTORY))
    {
        // Directory doesn't exist, create it
        _mkdir(SNAPDIR);
    }

    while (!done && counter <= 9999)
    {
#ifdef SNAP_JPEG
        sprintf(filename, "%s\\%s%04d.jpg", SNAPDIR, SNAPBASE, counter);
#else
        sprintf(filename, "%s\\%s%04d.pcx", SNAPDIR, SNAPBASE, counter);
#endif
        fp = fopen(filename, "rb");
        if (fp)
        {
            fclose(fp);
            counter++;
        }
        else
        {
            done = 1;
        }
    }

    if (counter <= 9999) // just in case...
    {
#ifdef SNAP_JPEG
        save_jpeg(filename, bmp, pal);
#else
        save_pcx(filename, bmp, pal);
#endif
    }
}

void screen_snap(void)
{
    MYBITMAP* bmp = create_bitmap(SCREEN_W, SCREEN_H);
    if (bmp == NULL) return;

    blit(screen, bmp, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

    // Use your palette implementation
    RGB* current_palette = get_current_palette();
    snap(bmp, current_palette);
    destroy_bitmap(bmp);
}