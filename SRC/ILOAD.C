// iload.c
//
//  The rom driver loader functions
//
//  December, 1998
//  jerry@mail.csh.rit.edu

#include <stdlib.h>
#include <stdbool.h>
#include "../INCLUDE/allegro.h"
#include "../INCLUDE/general.h"

#include "../INCLUDE/iload.h"

/*
[DataRoms]
Rom1 = 0    4096  pacman.6e
Rom2 = 4096    4096  pacman.6f
Rom3 = 8192    4096  pacman.6h
Rom4 = 12288    4096  pacman.6j

[Map1]
MapType = pacman
StartAddress = 3436
OverlayAddress = 35B5
OverlayCount = 240
Width = 28
Height = 32
Gfx_Bank = 1
HC1 = 1   4
HC2 = 26   4
HC3 = 1  24
HC4 = 26  24
*/

#if 0

#define MAP_TYPE_NONE    (0)
#define MAP_TYPE_PACMAN  (1)

typedef struct map_data{
    int map_type;
    int Gfx_Bank;
    long width;
    long height;
    long Start_Address;
    long Overlay_Address;
    long Overlay_Count;   // eventually - OverlayCountAddress
    int hc[8]; // hardcoded info - eventually hc_address[]
} MAP_DATA;
#endif

BOOL DataRomsLoaded = FALSE;

BOOL MapDataAvailable = FALSE;
BOOL TextDataAvailable = FALSE;

void Data_Driver_Sanity_Check(void)
{
}

void Load_Data_Roms(void)
{
}

void Free_Data_Roms(void)
{
}

void Get_Map_Driver_Data(void)
{
}

void Get_Text_Driver_Data(void)
{
}
