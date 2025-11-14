// inidriv.c
//
//  Ini driver loader functions...
//
//  October, 1998
//
//   Ivan Mackintosh

#include "../INCLUDE/allegro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "../INCLUDE/sprtplte.h"
#include "../INCLUDE/general.h"
#include "../INCLUDE/gamedesc.h"
#include "../INCLUDE/inidriv.h"
#include "../INCLUDE/config.h"
#include "../INCLUDE/coding.h"
#include "../INCLUDE/editmode.h"
#include "../INCLUDE/DRIVSEL.H"
#include "../INCLUDE/guipal.h"

// when the specified rom is located its path is stored here.
static char ROMPathFound[1024] = {0};

// still called age for easy upgradability from age
const char * BackupDir = "AGEBAK";
const char * PatchDir = "AGEPATCH";

const int ALTERNATE_ROMS_DETECT = 0x80000000;
const int ALTERNATE_ROMS_SIZE = 0x7fffffff;

// forward references
BOOL ReadColourPalettes(void);
int IndexFromNumColours(int Cols);
BOOL get_config_int_array(char * Section, char * Label,
                          int * PaletteVals, int * argc);

extern MYBITMAP* create_bitmap(int width, int height);
extern int alert(const char*, const char*, const char*, const char*, const char*, int, int);

// Display a given error code on a dialog in the middle of the screen
void DisplayError(const char * pFormatStr, ...)
{
   char     FormatStrBuffer[100] = { 0 };
   va_list  msgArgs;

   /* Convert the parameters to an actual message and write it into
    * FormatStrBuffer. */
   va_start (msgArgs, pFormatStr);
   vsprintf (FormatStrBuffer, pFormatStr, msgArgs);
   va_end (msgArgs);

//   printf("Error - %s",FormatStrBuffer);
   not_busy();
   alert("Error", FormatStrBuffer, NULL, "Continue", NULL, 0, 0);
}


// given a name of a file check the rompath to see if the file exists
int ScanROMPath(const char * Path)
{
   int i = 0;

   // no path?
   if (Path[0] == '\0')
      return -1;

   // find end position
   while ((Path[i] != '\0') && (Path[i] != ';'))
      i ++;
   
   return i;
}

int FindRomPath(char* ROMName)
{
    int Terminator = 0;
    int Pos = 0;
    char tmpFullPath[256];

    printf("\n=== FindRomPath DEBUG ===\n");
    printf("Searching for: %s\n", ROMName);
    printf("ROMPath: %s\n", ROMPath);

    for (;;)
    {
        Terminator = ScanROMPath(&ROMPath[Pos]);
        if (Terminator == -1)
        {
            printf("End of ROMPath reached - ROM not found!\n");
            printf("=========================\n\n");
            return 1;
        }

        strncpy(tmpFullPath, &ROMPath[Pos], Terminator);
        tmpFullPath[Terminator] = 0;
        put_backslash(tmpFullPath);
        strcat(tmpFullPath, ROMName);

        printf("  Trying: %s ... ", tmpFullPath);

        // found it - return success
        if (file_exists(tmpFullPath, 0, NULL) != 0)
        {
            printf("FOUND!\n");
            strncpy(ROMPathFound, &ROMPath[Pos], Terminator);
            ROMPathFound[Terminator] = 0;
            printf("ROMPathFound set to: %s\n", ROMPathFound);
            printf("=========================\n\n");
            return 0;
        }
        else
        {
            printf("NOT FOUND\n");
        }

        if (ROMPath[Pos + Terminator] == ';')
            Terminator++;

        Pos += Terminator;
    }
}

/* default palette - used if one doesn't exist in the ini driver */
static RGB DefaultColours[] =
{
	{0x00,0x00,0x00}, {0x3f,0x00,0x00}, {0x00,0x3f,0x00}, {0x00,0x00,0x3f},
	{0x3f,0x3f,0x00}, {0x3f,0x00,0x3f}, {0x00,0x3f,0x3f}, {0x3f,0x3f,0x3f},
	{0x08,0x08,0x08}, {0x10,0x10,0x10}, {0x18,0x18,0x18}, {0x20,0x20,0x20},
	{0x28,0x28,0x28}, {0x30,0x30,0x30}, {0x38,0x38,0x38}, {0x3f,0x3f,0x3f}
};

// Debug function to check ROM file loading
static void debug_rom_loading(void) {
    printf("\n=== DEBUG ROM LOADING ===\n");
    printf("Number of ROMs: %d\n", NumGfxRoms);

    for (int i = 0; i < NumGfxRoms; i++) {
        printf("ROM %d:\n", i + 1);
        printf("  Load Address: 0x%lX\n", GfxRoms[i].LoadAddress);
        printf("  Size: %ld bytes\n", GfxRoms[i].Size);
        printf("  Alternate: %s\n", GfxRoms[i].Alternate ? "YES" : "NO");
        printf("  ROM Name: '%s'\n", GfxRoms[i].ROMName);

        // Check if file exists
        if (exists(GfxRoms[i].ROMName)) {
            printf("  FILE EXISTS: YES\n");
        }
        else {
            printf("  FILE EXISTS: NO - looking for: %s\n", GfxRoms[i].ROMName);

            // Try to find the file in common locations
            char possible_paths[3][256];
            sprintf(possible_paths[0], "roms/%s", GfxRoms[i].ROMName);
            sprintf(possible_paths[1], "../roms/%s", GfxRoms[i].ROMName);
            sprintf(possible_paths[2], "./%s", GfxRoms[i].ROMName);

            for (int j = 0; j < 3; j++) {
                if (exists(possible_paths[j])) {
                    printf("  FOUND AT: %s\n", possible_paths[j]);
                    printf("  File size: %ld bytes\n", file_size(possible_paths[j]));
                    break;
                }
            }
        }
 //       printf("  File size: %ld bytes\n", file_size(GfxRoms[i].ROMName));
        
    }
    printf("=== END DEBUG ===\n\n");
}

// called only from ReadINIFileInfo for reading in the information from the
// ini file and storing it globally
static BOOL ReadFromDriverIniFile(void)
{
    char* pBuffer = NULL;
    char Buffer[255];
    int i, k;
    int argc;
    char** argv;

    pBuffer = get_config_string("General", "Description", "None");
    if (strcmp(pBuffer, "None") == 0)
    {
        DisplayError("Can't load .ini driver!"); return FALSE;
    }


    strncpy(GameDescription, pBuffer, MAX_GAME_DESCRIPTION);
    GameDescription[MAX_GAME_DESCRIPTION - 1] = 0;

    if ((NumGfxBanks = get_config_int("Layout", "GfxDecodes", 0)) == 0)
    {
        DisplayError("INI file error 'GfxDecodes'"); return FALSE;
    }

    // if this fail the orientation will be 0 - which is OK.
    Orientation = get_config_int("Layout", "Orientation", 0);

    GfxBanks = malloc(NumGfxBanks * sizeof(SPRITE_PALETTE));
    if (GfxBanks == NULL)
    {
        DisplayError("GfxBanks malloc failed"); return FALSE;
    }

    GfxBankExtraInfo = malloc(NumGfxBanks * sizeof(GFXBANKEXTRA));
    if (GfxBankExtraInfo == NULL)
    {
        free(GfxBanks);
        DisplayError("GfxBanksExtra malloc failed");
        return FALSE;
    }

    for (i = 0; i < NumGfxBanks; i++)
    {
        char GfxBank[10];
        int xoffset_count, yoffset_count;

        sprintf(GfxBank, "Decode%d", i + 1);

        // read staight forward stuff from INI file
        GfxBanks[i].sprite_w = get_config_int(GfxBank, "width", -1);
        GfxBanks[i].sprite_h = get_config_int(GfxBank, "height", -1);
        GfxBanks[i].n_total = get_config_int(GfxBank, "total", -1);
        GfxBanks[i].first_sprite = 0; // set the position to the default
        GfxBankExtraInfo[i].planes = get_config_int(GfxBank, "planes", -1);
        GfxBankExtraInfo[i].startaddress = get_config_int(GfxBank, "start", -1);
        GfxBankExtraInfo[i].charincrement = get_config_int(GfxBank, "charincrement", -1);

        // did any fail?
        if ((GfxBanks[i].sprite_w == -1) || (GfxBanks[i].sprite_h == -1) ||
            (GfxBanks[i].n_total == -1) || (GfxBankExtraInfo[i].planes == -1) ||
            (GfxBankExtraInfo[i].startaddress == -1) || (GfxBankExtraInfo[i].charincrement == -1))
        {
            free(GfxBanks);
            free(GfxBankExtraInfo);
            DisplayError("INI file corrupt");
            return FALSE;
        }

        // read plane offsets and store in array
        argv = get_config_argv(GfxBank, "planeoffsets", &argc);
        // sanity check
        if (argc != GfxBankExtraInfo[i].planes)
        {
            free(GfxBanks);
            free(GfxBankExtraInfo);
            DisplayError("INI file corrupt - planeoffsets");
            return FALSE;
        }

        // copy planeoffsets in to array
        for (k = 0; k < argc; k++)
            GfxBankExtraInfo[i].planeoffsets[k] = atol(argv[k]);

        // Free the argv array for planeoffsets
        for (k = 0; k < argc; k++) {
            if (argv[k]) free(argv[k]);
        }
        free(argv);

        // read xoffsets and store in array - FIXED: use 64 as max_count (array size)
        xoffset_count = get_config_int_array(GfxBank, "xoffsets", GfxBankExtraInfo[i].xoffset, 64);
        // sanity check
        if (xoffset_count != GfxBanks[i].sprite_w)
        {
            free(GfxBanks);
            free(GfxBankExtraInfo);
            DisplayError("INI file corrupt - xoffsets");
            return FALSE;
        }

        // read yoffsets and store in array - FIXED: use 64 as max_count (array size)
        yoffset_count = get_config_int_array(GfxBank, "yoffsets", GfxBankExtraInfo[i].yoffset, 64);
        // sanity check
        if (yoffset_count != GfxBanks[i].sprite_h)
        {
            free(GfxBanks);
            free(GfxBankExtraInfo);
            DisplayError("INI file corrupt - yoffsets");
            return FALSE;
        }
    }

    // calculate the number of graphics roms
    NumGfxRoms = 0;
    do
    {
        NumGfxRoms++;
        sprintf(Buffer, "Rom%d", NumGfxRoms);
        pBuffer = get_config_string("GraphicsRoms", Buffer, "None");
    } while (strcmp(pBuffer, "None") != 0);
    NumGfxRoms--;

    // is the ini file corrupt?
    if (NumGfxRoms == 0)
    {
        free(GfxBanks);
        free(GfxBankExtraInfo);
        DisplayError("INI file corrupt - graphicsroms");
        return FALSE;
    }

    // allocate memory for appropriate number of structures;
    GfxRoms = malloc(NumGfxRoms * sizeof(GFXROM));
    if (GfxRoms == NULL)
    {
        free(GfxBanks);
        free(GfxBankExtraInfo);
        DisplayError("GfxRoms malloc failed");
        return FALSE;
    }

    // read the information in to the array
    for (i = 0; i < NumGfxRoms; i++)
    {
        // read rom info and store in array
        sprintf(Buffer, "Rom%d", i + 1);
        argv = get_config_argv("GraphicsRoms", Buffer, &argc);
        // sanity check
        if (argc != 3) // loadaddress,size,name
        {
            free(GfxBanks);
            free(GfxBankExtraInfo);
            free(GfxRoms);
            DisplayError("INI file corrupt - %s", Buffer);
            return FALSE;
        }

        // copy info in to array
        GfxRoms[i].LoadAddress = atol(argv[0]);
        GfxRoms[i].Size = atol(argv[1]);
        GfxRoms[i].Alternate = FALSE;
        // detect for Alternate byte loading flag
        if ((GfxRoms[i].Size & ALTERNATE_ROMS_DETECT) == ALTERNATE_ROMS_DETECT)
        {
            GfxRoms[i].Size &= ALTERNATE_ROMS_SIZE;
            GfxRoms[i].Alternate = TRUE;
        }

        strncpy(GfxRoms[i].ROMName, argv[2], MAX_ROM_NAME);
        GfxRoms[i].ROMName[MAX_ROM_NAME - 1] = 0;

        // Free the argv array for GraphicsRoms
        for (k = 0; k < argc; k++) {
            if (argv[k]) free(argv[k]);
        }
        free(argv);
    }

    // Debug ROM loading
    debug_rom_loading();

    // finally read in the colour palettes or set defaults if none
    // exist
    if (ReadColourPalettes() == FALSE)
    {
        // free any allocated colour palettes
        for (i = 0; i < MAX_COL_PLANES; i++)
        {
            if (ColPalettes[i] != NULL)
            {
                free(ColPalettes[i]);
                ColPalettes[i] = NULL;
                NumColPalettes[i] = 0;
            }
        }
        free(GfxBanks);
        free(GfxBankExtraInfo);
        free(GfxRoms);
        return FALSE;
    }

    return TRUE;
}


BOOL ReadColourPalettes(void)
{
    int i, j;
    int palette_count;  // Changed from argc to palette_count for clarity
    int PaletteVals[200];
    BOOL DefaultPaletteUsed[MAX_COL_PLANES];

    for (i = 0; i < MAX_COL_PLANES; i++)
        DefaultPaletteUsed[i] = FALSE;

    // define defaults to any colour planes used - these will
    // be overwritten if any exist in the ini file
    for (i = 0; i < NumGfxBanks; i++)
    {
        int BankPlanes = GfxBankExtraInfo[i].planes;
        if (NumColPalettes[BankPlanes] == 0)
        {
            int NumColours = 1 << BankPlanes;
            ColPalettes[BankPlanes] = malloc(sizeof(RGB) * NumColours);
            if (ColPalettes[BankPlanes] == NULL)
            {
                DisplayError("Malloc failed creating default palettes");
                return FALSE;
            }
            memcpy(ColPalettes[BankPlanes], DefaultColours,
                NumColours * sizeof(RGB));
            NumColPalettes[BankPlanes] = 1;
            DefaultPaletteUsed[BankPlanes] = TRUE;
        }
    }

    // read in all of the palettes that exist in the ini file
    i = 0;
    for (;;)
    {
        char PaletteNum[12];
        int NumCols = 0;
        int Index;

        sprintf(PaletteNum, "Palette%d", i + 1);
        i++;

        // FIXED: Use the actual array size (200) and get the return value
        palette_count = get_config_int_array("Palette", PaletteNum, PaletteVals, 200);

        // palette didn't exist
        if (palette_count == 0)
            break;

        // sanity checking
        NumCols = PaletteVals[0];
        Index = IndexFromNumColours(NumCols);

        // do we have default for this colour plane?
        if (NumColPalettes[Index] == 0)
        {
            // no default therefore it doesn't belong to this graphics set
            DisplayError("Palettes Corrupt, Cols = %d, %d", NumCols, Index);
            return FALSE;
        }

        // are there the right number of colours?
        if ((NumCols * 3) != (palette_count - 1))
        {
            // each colour should have 3 values. The ini file should store
            // the numcolours followed by the 3 * numcolours for each rgb
            DisplayError("Palettes Corrupt, palette_count = %d", palette_count);
            return FALSE;
        }


        if (DefaultPaletteUsed[Index] == TRUE)
        {
            // we have already malloced a default palette here so overwrite it

            // ensure we don't try and overwrite it again
            DefaultPaletteUsed[Index] = FALSE;
        }
        else
        {
            // add a new colour palette
            int PalSize = NumCols * sizeof(RGB);

            // create memory for larger palette
            RGB* tPal = malloc(PalSize * (NumColPalettes[Index] + 1));
            if (tPal == NULL)
            {
                DisplayError("Malloc failed creating palette");
                return FALSE;
            }

            // copy old palette to new memory
            memcpy(tPal, ColPalettes[Index], PalSize * NumColPalettes[Index]);
            NumColPalettes[Index]++;

            // free old memory and copy pointer to new palette
            free(ColPalettes[Index]);
            ColPalettes[Index] = tPal;
        }

        for (j = 0; j < NumCols; j++)
        {
            int Offset = NumCols * (NumColPalettes[Index] - 1);
            ColPalettes[Index][j + Offset].r = PaletteVals[(j * 3) + 1];
            ColPalettes[Index][j + Offset].g = PaletteVals[(j * 3) + 2];
            ColPalettes[Index][j + Offset].b = PaletteVals[(j * 3) + 3];
        }

    } // end of for(;;)

    return TRUE;
}


// to calculate this return the number of right shifts required before
// the parameter = 1
int IndexFromNumColours(int Cols)
{
   int Count = 0;

   while (Cols != 1)
   {
      Cols = Cols >> 1;
      Count ++;
   }

   return Count;
}



BOOL LoadGfxRomData(void)
{
    int i;
    FILE* fp;
    char ROMDirWithName[256];
    int total = 0;

    // find the rom path. Use the first rom name from the set
    sprintf(ROMDirWithName, "%s/%s", ROMDirName, GfxRoms[0].ROMName);
    printf("DEBUG: Looking for ROM path with: %s\n", ROMDirWithName);

    if (FindRomPath(ROMDirWithName) != 0)
    {
        DisplayError("%s not found in ROM paths", ROMDirWithName);
        printf("ERROR: ROM path not found for: %s\n", ROMDirWithName);
        printf("ROMPath setting: %s\n", ROMPath);
        return FALSE;
    }

    printf("ROM path found: %s\n", ROMPathFound);

    // allocate the memory to store the roms
    for (i = 0; i < NumGfxRoms; i++)
        total += GfxRoms[i].Size;

    GfxRomData = malloc(total);
    if (GfxRomData == NULL)
    {
        DisplayError("GfxRomData malloc failed");
        return FALSE;
    }

    for (i = 0; i < NumGfxRoms; i++)
    {
        char FullPath[256];

        // build full path including filename
        strcpy(FullPath, ROMPathFound);
        put_backslash(FullPath);
        strcat(FullPath, ROMDirName);
        put_backslash(FullPath);
        strcat(FullPath, GfxRoms[i].ROMName);

        printf("Trying to open ROM %d: %s\n", i + 1, FullPath);

        if ((fp = fopen(FullPath, "rb")) == NULL)
        {
            /* file not found */
            free(GfxRomData);
            DisplayError("File not found: %s", FullPath);
            printf("ERROR: Could not open: %s\n", FullPath);
            return FALSE;
        }

        printf("Successfully opened: %s\n", FullPath);

        if (GfxRoms[i].Alternate == FALSE)
        {
            size_t bytes_read = fread(&GfxRomData[GfxRoms[i].LoadAddress], 1,
                GfxRoms[i].Size, fp);
            if (bytes_read != GfxRoms[i].Size)
            {
                /* bad load */
                free(GfxRomData);
                DisplayError("ROM Size load error: expected %ld, got %zu",
                    GfxRoms[i].Size, bytes_read);
                fclose(fp);
                return FALSE;
            }
            printf("Loaded %zu bytes at address 0x%lX\n", bytes_read, GfxRoms[i].LoadAddress);
        }
        else
        {
            long t;
            for (t = 0; t < GfxRoms[i].Size; t++)
            {
                long Address = GfxRoms[i].LoadAddress + (t * 2);
                GfxRomData[Address] = fgetc(fp);
            }
            printf("Loaded %ld bytes (alternate mode) at address 0x%lX\n",
                GfxRoms[i].Size, GfxRoms[i].LoadAddress);
        }

        fclose(fp);
    }

    printf("All ROMs loaded successfully!\n");
    return TRUE;
}


BOOL AllocateGfxBanks(void)
{
   int i, j;
   
   // create the bitmaps inorder to store the graphics banks
   for (i = 0; i < NumGfxBanks; i ++)
   {
      GfxBanks[i].bmp = create_bitmap(GfxBanks[i].sprite_w * GfxBanks[i].n_total,GfxBanks[i].sprite_h);
      if (GfxBanks[i].bmp == NULL)
      {
         for (j = 0; j < NumGfxBanks - 1; j ++)
         {
            // failed to create, destroy any previously created ones
            destroy_bitmap(GfxBanks[j].bmp);
         }
	// these should be outside, ivan...
	DisplayError("CreateBitmap failed");
	return FALSE;
      }
   }
   
   return TRUE;
}


// assumes inifilename includes fully qualified path
BOOL LoadDriver(char * INIFileName)
{
   BOOL retval;
   char INIDriverName[128];
   char * fname = get_filename(INIFileName);
   int i = 0;

   // free any currently loaded driver
   FreeDriver();
   
   /* setup INI Driver name */
   // JERR -- since the file selector uses a full path, we lose this next line.
   // when i write a new file loader dialog, this will change back...
   //sprintf(INIDriverName, "drivers/%s", fname);

   sprintf(INIDriverName, "%s", fname);
   
   /* set up ROMDirName - ini filename without extension */
   while ((fname[i] != '\0') && (fname[i] != '.'))
   {
      ROMDirName[i] = fname[i];
      i ++;
   }
   ROMDirName[i] = 0;

   // load the data from the INI file
   push_config_state();

   //set_config_file(INIDriverName);
   set_config_file(INIFileName);

   sprintf(INI_Driver_Path, "%s", INIFileName);

   retval = ReadFromDriverIniFile();
   
   pop_config_state();

   // did the INI read fail?
   if (retval == FALSE)
      return retval;

   // load graphics roms into memory
   retval = LoadGfxRomData();

   // if load failed then free all INI Driver memory and fail
   if (retval == FALSE)
   {

      if(GfxBanks)         free(GfxBanks);
      if(GfxBankExtraInfo) free(GfxBankExtraInfo);
      if(GfxRoms)          free(GfxRoms);
      return retval;
   }

   // create bitmaps for graphics banks
   retval = AllocateGfxBanks();

   // if load failed then free all INI Driver memory and loaded roms and fail
   if (retval == FALSE)
   {
      if(GfxBanks)         free(GfxBanks);
      if(GfxBankExtraInfo) free(GfxBankExtraInfo);
      if(GfxRoms)          free(GfxRoms);
      if(GfxRomData)       free(GfxRomData);
      return retval;
   }

   // now decode the graphics banks
   SwitchGraphicsBank(-1, 0);
   
   // success
   GameDriverLoaded = TRUE;
   
   return retval;
}




void FreeDriver(void)
{
   // no need to free up if none loaded
   if (GameDriverLoaded == TRUE)
   {
      int i;

      // get rid of the bitmaps first
      for (i = 0; i < NumGfxBanks; i ++)
         destroy_bitmap(GfxBanks[i].bmp);
      
      if(GfxBanks)         free(GfxBanks);
      if(GfxBankExtraInfo) free(GfxBankExtraInfo);
      if(GfxRoms)          free(GfxRoms);
      if(GfxRomData)       free(GfxRomData);

      // free colour palettes
      for (i = 0; i < MAX_COL_PLANES; i ++)
      {
         if (ColPalettes[i] != NULL)
         {
            free(ColPalettes[i]);
            ColPalettes[i] = NULL;
            NumColPalettes[i] = 0;
         }
      }
      
      // clear any variables left behind from the driver
      InitialiseGameDesc();
   }
}



// Function to parse integer arrays from config (used for planeoffsets, xoffsets, yoffsets, etc.)
int get_config_int_array(const char* section, const char* name, int* array, int max_count) {
    int argc;
    char** argv = get_config_argv(section, name, &argc);

    printf("get_config_int_array: [%s] %s, argc = %d, max_count = %d\n",
        section, name, argc, max_count);

    if (!argv || argc == 0) {
        printf("ERROR: No arguments found for [%s] %s\n", section, name);
        return 0;
    }

    // Sanity check on max_count - if it's unreasonably large, use a safe limit
    if (max_count > 1000 || max_count < 0) {
        printf("WARNING: Suspicious max_count (%d), using argc (%d) as limit\n", max_count, argc);
        max_count = argc;
    }

    // Validate we have enough arguments but not too many
    if (argc > max_count) {
        printf("WARNING: Too many arguments (%d) for [%s] %s, truncating to %d\n",
            argc, section, name, max_count);
        argc = max_count;
    }

    // Parse each argument
    int count = 0;
    for (int i = 0; i < argc && count < max_count; i++) {
        if (argv[i] && strlen(argv[i]) > 0) {
            array[count] = atoi(argv[i]);
            printf("  array[%d] = %d (from '%s')\n", count, array[count], argv[i]);
            count++;
        }
    }

    // Free the argv array
    for (int i = 0; i < argc; i++) {
        if (argv[i]) free(argv[i]);
    }
    free(argv);

    printf("get_config_int_array: parsed %d values\n", count);
    return count;
}


/* Copies a file. filename includes relative path */
/* */
BOOL CopyROM(char * from, char * to, long size, int CommandLine)
{
	FILE * fp_from;
	FILE * fp_to;
	char Buffer[1024];
	long tot = 0;
	int i = 0;

	if ((fp_from = fopen(from, "rb")) == NULL)
	{
      DisplayError("Couldn't open ROM for backup");
		return FALSE;
	}

	if ((fp_to = fopen(to, "wb")) == NULL)
	{
      DisplayError("Couldn't create backup");
		return FALSE;
	}

	do
	{
		i = fread(Buffer, 1, sizeof(Buffer), fp_from);

		if (i == 0 && tot != size)
		{
         DisplayError("Couldn't copy ROM");
			fclose(fp_from);
			fclose(fp_to);
			return FALSE;
		}

		if (i > 0)
			fwrite(Buffer, 1, i, fp_to);

		tot += i;

	} while (i > 0);

	fclose(fp_from);
	fclose(fp_to);
	return TRUE;
}


void MakeBackupPath(const char * DirName, char * Buffer, char * Filename, int AppendFilename)
{
   strcpy(Buffer, ROMPathFound); // add the full rom path
   put_backslash(Buffer);
   strcat(Buffer, ROMDirName); // now add the directory name
   put_backslash(Buffer);
   strcat(Buffer, DirName); // now add agebak/agepatch etc.
   
	/* if caller also wants filename then append that too */
	if (AppendFilename)
	{
      put_backslash(Buffer);
		strcat(Buffer, Filename);
	}
}


/* To avoid original roms being overwritten a subdirectory called agebak */
/* is created and a copy of all of the original roms is store there. */
/* If agebak already exists then nothing is copied to avoid overwriting */
/* the back ups of the originals. */
BOOL MakeBackups(void)
{
	char BackupPath[100];
   char FromPath[100];
	FILE * fp;
	int i;

	MakeBackupPath(BackupDir, BackupPath, GfxRoms[0].ROMName, 1);

	/* are the roms already backed up? (can we open one for read) */
	if ((fp = fopen(BackupPath, "rb")) != NULL)
	{
		fclose(fp);
		return TRUE;
	}

	/* at this point we need to make backups */

	/* first create directory */
	MakeBackupPath(BackupDir, BackupPath, GfxRoms[0].ROMName, 0);
    _mkdir(BackupPath);

	/* back up the roms */
	for (i = 0; i < NumGfxRoms; i ++)
	{
      strcpy(FromPath, ROMPathFound);
      put_backslash(FromPath);
      strcat(FromPath, ROMDirName);
      put_backslash(FromPath);
      strcat(FromPath, GfxRoms[i].ROMName);

		MakeBackupPath(BackupDir, BackupPath, GfxRoms[i].ROMName, 1);
		if (CopyROM(FromPath, BackupPath, GfxRoms[i].Size, 0) == FALSE)
			return FALSE;
	}

	return TRUE;
}



// Saves the romset out to disk. Attempts to backup originals if
// a backup doesn't already exist.
BOOL SaveDriver(void)
{
	FILE * fp = NULL;
	int i = 0;
   char FullPath[100] = {0};
   
	/* try and make the back ups */
	if (!MakeBackups())
		return FALSE;

	for (i = 0; i < NumGfxRoms; i ++)
	{
      strcpy(FullPath, ROMPathFound);
      put_backslash(FullPath);
      strcat(FullPath, ROMDirName);
      put_backslash(FullPath);
      strcat(FullPath, GfxRoms[i].ROMName);

		if ((fp = fopen(FullPath, "wb")) == NULL)
		{
			/* cant open file for write */
			DisplayError("Failed to open for write");
			return FALSE;
		}
      if (GfxRoms[i].Alternate == FALSE)
      {
         if (fwrite(&GfxRomData[GfxRoms[i].LoadAddress], 1, GfxRoms[i].Size, fp)
             != GfxRoms[i].Size)
         {
            /* bad save */
            DisplayError("Failed to save");
            fclose(fp);
            return FALSE;
         }
      }
      else
      {
         long t;
         for (t = 0; t < GfxRoms[i].Size; t++)
         {
            long Address = GfxRoms[i].LoadAddress + (t * 2);
            fputc(GfxRomData[Address],fp);
         }
      }

		fclose(fp);
	}

   return TRUE;
}




void WritePatchChanges(long StartSeq, long SeqLen, FILE * fpModifiedRom, FILE * fpPatchRom)
{
	int i;

	/* write value of j as 24bit number */
	char * p = (char *)&StartSeq;
	fputc(p[2], fpPatchRom);
	fputc(p[1], fpPatchRom);
	fputc(p[0], fpPatchRom);

	/* write value of SeqLen as 16bit number */
	p = (char *)&SeqLen;
	fputc(p[1], fpPatchRom);
	fputc(p[0], fpPatchRom);

	/* rewind fpModifiedRom to start of change */
	fseek(fpModifiedRom, StartSeq, SEEK_SET);

	/* copy bytes */
	for (i = 0; i < SeqLen; i ++)
	{
		BYTE ch = fgetc(fpModifiedRom);
		fputc(ch, fpPatchRom);
	}

	/* make fpModifiedRom in the same position as when we entered this function */
	fgetc(fpModifiedRom);
}


void DoComparison(char * ROMName, long Size)
{
	char BackupPath[100] = {0};
	char PatchPath[100] = {0};
   char FullPath[100] = {0};
	FILE * fpModifiedRom = NULL;
	FILE * fpBackupRom = NULL;
	FILE * fpPatchRom = NULL;
	long StartSeq = -1, SeqLen = 0, j;
	int HeaderWritten = 0;

	/* open the backup ROM (do this first as it is the most likely to fail) */
	MakeBackupPath(BackupDir, BackupPath, ROMName, 1);
	if ((fpBackupRom = fopen(BackupPath, "rb")) == NULL)
	{
		/* just be quiet about this as it only means that the user hasn't edited */
		/* either the maps or the graphics therefore there is no backup for them */
		/* and no patch needed anyway */
		return;
	}

	/* now open the modified rom */
   strcpy(FullPath, ROMPathFound);
   put_backslash(FullPath);
   strcat(FullPath, ROMDirName);
   put_backslash(FullPath);
   strcat(FullPath, ROMName);

	if ((fpModifiedRom = fopen(FullPath, "rb")) == NULL)
	{
		printf("ERROR: Failed to open modified rom \"%s\"\n skipping", ROMName);
		fclose(fpBackupRom);
		return;
	}

	/* on to the comparison and creating the IPS file */
	/* Compare Roms */
	StartSeq = -1;

	for (j = 0; j < Size; j ++)
	{
		BYTE BackupCh = fgetc(fpBackupRom);
		BYTE ModifiedCh = fgetc(fpModifiedRom);

		/* do bytes match? */
		if (BackupCh != ModifiedCh)
		{
			/* set up our start flag if it is not already done */
			if (StartSeq == -1)
			{
				StartSeq = j;
				SeqLen = 0;
				if (!HeaderWritten)
				{
					/* create patch file */
					MakeBackupPath(PatchDir, PatchPath,ROMName, 1);
					if ((fpPatchRom = fopen(PatchPath, "wb")) == NULL)
					{
						/* cant open file for write */
						printf("ERROR: Failed to create patch file: \"%s\" skipping\n", PatchPath);
						fclose(fpBackupRom);
						fclose(fpModifiedRom);
						return;
					}

					fwrite("PATCH", 1, 5, fpPatchRom);
					HeaderWritten = 1;
				}
			}

			SeqLen ++;
			/* don't allow to overflow 2 bytes */
			if (SeqLen == 65535)
			{
				WritePatchChanges(StartSeq, SeqLen, fpModifiedRom, fpPatchRom);
				StartSeq = -1;
				SeqLen = 0;
			}
		}
		else
		{
			/* bytes match - do we need to write any patch file */
			if (StartSeq != -1)
			{
				WritePatchChanges(StartSeq, SeqLen, fpModifiedRom, fpPatchRom);
				StartSeq = -1;
				SeqLen = 0;
			}
		}
	}

	/* do we need to write trailer? */
	if (HeaderWritten)
	{
		/* have we written all of the changes */
		if (StartSeq != -1)
		{
			WritePatchChanges(StartSeq, SeqLen, fpModifiedRom, fpPatchRom);
			StartSeq = -1;
			SeqLen = 0;
		}
		fwrite("EOF", 1, 3, fpPatchRom);
		HeaderWritten = 0;
		fclose(fpPatchRom);
	}

	fclose(fpBackupRom);
}


/* Make IPS patch file for specified game and put them in the AGEPATCH */
/* directory. This is only called from Command line so we can use printf */
/* for errors */
void MakePatch(void)
{
	char PatchPath[100] = {0};
	int i = 0;

	/* create patch directory */
	MakeBackupPath(PatchDir, PatchPath, GfxRoms[0].ROMName, 0);
    _mkdir(PatchPath);

	/* first do graphics roms */
	for (i = 0; i < NumGfxRoms; i ++)
	{
      // build full path including filename
		DoComparison(GfxRoms[i].ROMName, GfxRoms[i].Size);
	}

#if 0
	/* are there any map roms to create patch files for? */
	if (numDataRoms != 0)
	{
		for (i = 0; i < numDataRoms; i ++)
		{
			DoComparison(DataRoms[i].ROMName, DataRoms[i].Size);
		}
	}
#endif
}

// saves the current palette
void SavePalette(int NumCols, int PalIndex, int CurrentPal, int IniFilePalNum)
{
   char * Buffer;
   char NumBuf[10];
   int j;

   // This buffer will hold number like the following:
   // 4 255 255 255  255 255 255
   // Note the 10 allows for the first number, space and a bit of overhead
   // the 13 allows for "255 255 255  ".
   // This is dynamically allocated as we don't know how much space is required
   // until we know the number of colours
   Buffer = malloc(10 + (NumCols * 13));
   if (Buffer == NULL)
   {
      alert("Error", "Failed to save palette!", NULL,  "&Okay", NULL, 'O', 0);
      return;
   }
   sprintf(Buffer, "%d ", NumCols);

   for (j = 0; j < NumCols; j ++)
   {
      int Offset = NumCols * CurrentPal;

      sprintf(NumBuf, "%d ", ColPalettes[PalIndex][j + Offset].r);
      strcat(Buffer, NumBuf);
      sprintf(NumBuf, "%d ", ColPalettes[PalIndex][j + Offset].g);
      strcat(Buffer, NumBuf);
      sprintf(NumBuf, "%d  ", ColPalettes[PalIndex][j + Offset].b);
      strcat(Buffer, NumBuf);
   }

   // now write out the buffer
   sprintf(NumBuf, "Palette%d", IniFilePalNum);

   push_config_state();
   set_config_file(INI_Driver_Path);
   set_config_string("Palette", NumBuf, Buffer);
   pop_config_state();

   free (Buffer);
}

// traverses the list of all of the palette to write back to the
// ini file.
void SaveAllPalettes(void)
{
   int i,j;
   int IniFilePalNum = 1;

   // walk through each of the colour planes
   for (i = 0; i < MAX_COL_PLANES; i ++)
   {
      // walk through each palette for this plane
      for (j = 0; j < NumColPalettes[i]; j ++)
      {
         SavePalette((1 << i), i, j, IniFilePalNum);
         IniFilePalNum++;
      }
   }
}


void MergeFiles(char * ROMName, long size)
{
   char BackupPath[100] = {0};
   char PatchPath[100] = {0};
   FILE * fpNewRom = NULL;
   FILE * fpBackupRom = NULL;
   FILE * fpPatchRom = NULL;
   char Header[] = "PATCH";
   long Offset = 0;
   long NumBytes = 0;
   long i;
   long BackupPos = 0;
   BYTE ch;

   /* check for rom name in backup directory */
   MakeBackupPath(BackupDir, BackupPath,ROMName, 1);

   /* if not there then put it there */
   if ((fpBackupRom = fopen(BackupPath, "rb")) == NULL)
   {
      CopyROM(ROMName, BackupPath, size, 1);
      if ((fpBackupRom = fopen(BackupPath, "rb")) == NULL)
      {
         /* something has gone wrong with the backup abort this */
         /* rom merge. */
         return;
      }
   }

   /* at this point we should have a valid fpBackupRom */

   MakeBackupPath(PatchDir, PatchPath,ROMName, 1);
   if ((fpPatchRom = fopen(PatchPath, "rb")) == NULL)
   {
      /* Oh no! there is no patch file for this rom, just copy */
      /* backup file over original */

      fclose(fpBackupRom);
      CopyROM(BackupPath, ROMName, size, 1);
      return;
   }

   /* open the new rom file */
   if ((fpNewRom = fopen(ROMName, "wb")) == NULL)
   {
      /* abort */
      fclose(fpBackupRom);
      fclose(fpPatchRom);
      return;
   }

   /* check header */
   for (i = 0; i < 5; i ++)
   {
      BYTE ch = fgetc(fpPatchRom);
      if (ch != Header[i])
      {
         printf("Invalid Patch file \"%s\"", PatchPath);
         return;
      }
   } 

   /* right thats all of the tinkering about done now for the merge */
   for(;;)
   {
      Offset = 0;

      /* get the next 3 bytes for the offset */
      for (i = 0; i < 3; i ++)
      {
         ch = fgetc(fpPatchRom);
         Offset *= 256;
         Offset += ch;
      }

      /* end of file? */
      if (Offset == ('E' * 65536) + ('O' * 256) + 'F')
         break;

      /* now copy all of the bytes from the Backup to the new roms */
      /* up until the offset */

      fseek(fpBackupRom, BackupPos, SEEK_SET);
      for (i = BackupPos; i < Offset; i ++)
      {
         ch = fgetc(fpBackupRom);
         fputc(ch, fpNewRom);
      }

      /* how many bytes to merge? */
      ch = fgetc(fpPatchRom);
      NumBytes = (256 * ch) + fgetc(fpPatchRom);

      for (i = 0; i < NumBytes; i ++)
      {
         ch = fgetc(fpPatchRom);
         fputc(ch, fpNewRom);
      }

      BackupPos = Offset + NumBytes;
   }

   /* now copy any remaining bytes from the backup file */
   fseek(fpBackupRom, BackupPos, SEEK_SET);
   for (i = BackupPos; i < size; i ++)
   {
      ch = fgetc(fpBackupRom);
      fputc(ch, fpNewRom);
   }

   fclose(fpNewRom);
   fclose(fpBackupRom);
   fclose(fpPatchRom);
}


/* Patch IPS file with original roms */
void ApplyPatch(void)
{
   int i = 0;

   /* first do graphics roms */
   for (i = 0; i < NumGfxRoms; i ++)
   {
      MergeFiles(GfxRoms[i].ROMName,
                 GfxRoms[i].Size);
   }

#if 0
   /* are there any map roms to create patch files for? */
   if (numDataRoms != 0)
   {
      for (i = 0; i < numDataRoms; i ++)
      {
         MergeFiles(Drivers[DataRoms[i].ROMName,
                    Drivers[DataRoms[i].Size);
      }
   }
#endif
}
