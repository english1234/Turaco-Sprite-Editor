// gamedesc.c
//
// All the information required for the driver taken from
// the driver INI file
//
//  October, 1998
//
//   Ivan Mackintosh

#include <stdio.h>    // for sprintf
#include <stdbool.h>
//#include "allegro.h"
#include "../INCLUDE/sprtplte.h"
#include "../INCLUDE/general.h"
#include "../INCLUDE/gamedesc.h"

// have we loaded a driver?
BOOL GameDriverLoaded;

// the full pathname to the loaded driver
char INI_Driver_Path[255];

// the full name of the game taken from the Description field of the ini file
char GameDescription[MAX_GAME_DESCRIPTION];

int NumGfxBanks;
int Orientation;
char ROMDirName[MAX_ROM_NAME];

// array of sprite palettes each one containing a graphics bank and
// associated sprite info
SPRITE_PALETTE * GfxBanks;

// array of above type each one containing info about a graphics bank
GFXBANKEXTRA * GfxBankExtraInfo;


// the number of graphics roms used by this driver
int NumGfxRoms;

// array of above type each one containing information about a rom
GFXROM * GfxRoms;

// stores all of the graphics rom data ready to be decoded
// GFXROM struct is the indexes into this memory
unsigned char * GfxRomData;


// colour palettes for up to 8 colour planes
int NumColPalettes[MAX_COL_PLANES];
RGB * ColPalettes[MAX_COL_PLANES];


// initialise any crutial variables.
// must be called on startup
void InitialiseGameDesc(void)
{
   int i;
   
   GameDriverLoaded = FALSE;

   // ensure that palettes are zeroed -
   // assumes they have already been freed
   for (i = 0; i < MAX_COL_PLANES; i ++)
   {
      NumColPalettes[i] = 0;
      ColPalettes[i] = NULL;
   }

   NumGfxBanks = 0;
   Orientation = 0;
   NumGfxRoms  = 0;

   if(GfxBankExtraInfo) free(GfxBankExtraInfo);
   GfxBankExtraInfo = NULL;

   GameDescription[0] = '\0';
}


