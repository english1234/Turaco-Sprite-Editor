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
#include "../INCLUDE/editmenu.h"
#include "../INCLUDE/DRIVSEL.H"
#include "../INCLUDE/guipal.h"

// when the specified rom is located its path is stored here.
static char ROMPathFound[1024] = { 0 };

// still called age for easy upgradability from age
const char* BackupDir = "AGEBAK";
const char* PatchDir = "AGEPATCH";

const int ALTERNATE_ROMS_DETECT = 0x80000000;
const int ALTERNATE_ROMS_SIZE = 0x7fffffff;

// forward references
BOOL ReadColourPalettes(void);
int IndexFromNumColours(int Cols);
BOOL get_config_int_array(const char* Section, const char* Label, int* PaletteVals, int argc);

extern MYBITMAP* create_bitmap(int width, int height);
extern int alert(const char*, const char*, const char*, const char*, const char*, int, int);

// Display a given error code on a dialog in the middle of the screen
void DisplayError(const char* pFormatStr, ...)
{
    char     FormatStrBuffer[100] = { 0 };
    va_list  msgArgs;

    /* Convert the parameters to an actual message and write it into
     * FormatStrBuffer. */
    va_start(msgArgs, pFormatStr);
    vsprintf(FormatStrBuffer, pFormatStr, msgArgs);
    va_end(msgArgs);

    //   printf("Error - %s",FormatStrBuffer);
    not_busy();
    alert("Error", FormatStrBuffer, NULL, "Continue", NULL, 0, 0);
}


// given a name of a file check the rompath to see if the file exists
int ScanROMPath(const char* Path)
{
    int i = 0;

    // no path?
    if (Path[0] == '\0')
        return -1;

    // find end position
    while ((Path[i] != '\0') && (Path[i] != ';'))
        i++;

    return i;
}

int FindRomPath(char* ROMName)
{
    int Terminator = 0;
    unsigned int Pos = 0;
    char tmpFullPath[512];  // Increased buffer size
    char* rom_filename = NULL;
    char* last_slash = NULL;

    printf("\n=== FindRomPath DEBUG ===\n");
    printf("Searching for: %s\n", ROMName);
    printf("ROMPath: %s\n", ROMPath);
    printf("ROMPath length: %zu\n", strlen(ROMPath));

    // Extract just the filename from ROMName (in case it includes directory)
    last_slash = strrchr(ROMName, '/');
    if (!last_slash) last_slash = strrchr(ROMName, '\\');

    if (last_slash) {
        rom_filename = last_slash + 1;
        printf("ROM filename: %s\n", rom_filename);
    }
    else {
        rom_filename = ROMName;
    }

    for (;;)
    {
        Terminator = ScanROMPath(&ROMPath[Pos]);
        if (Terminator == -1)
        {
            printf("End of ROMPath reached - ROM not found!\n");
            printf("=========================\n\n");
            return 1;
        }

        printf("DEBUG: Path segment at Pos=%d, Terminator=%d: '", Pos, Terminator);
        // Print the segment for debugging
        for (int k = 0; k < Terminator && k < 50; k++) {
            printf("%c", ROMPath[Pos + k]);
        }
        printf("'\n");

        // Copy the path segment safely
        if (Terminator >= sizeof(tmpFullPath)) {
            printf("ERROR: Path segment too long: %d characters\n", Terminator);
            return 1;
        }

        memcpy(tmpFullPath, &ROMPath[Pos], Terminator);
        tmpFullPath[Terminator] = 0;

        printf("DEBUG: Copied path segment: '%s'\n", tmpFullPath);

        put_backslash(tmpFullPath, sizeof(tmpFullPath));

        // If ROMName includes directory, we need to append the full path
        if (last_slash) {
            // ROMName includes directory structure, append it directly
            if (strlen(tmpFullPath) + strlen(ROMName) < sizeof(tmpFullPath) - 1) {
                strcat(tmpFullPath, ROMName);
            }
            else {
                printf("ERROR: Path too long\n");
                return 1;
            }
        }
        else {
            // No directory in ROMName, use ROMDirName + ROMName
            if (strlen(tmpFullPath) + strlen(ROMDirName) + strlen(ROMName) + 2 < sizeof(tmpFullPath) - 1) {
                strcat(tmpFullPath, ROMDirName);
                put_backslash(tmpFullPath, sizeof(tmpFullPath));
                strcat(tmpFullPath, ROMName);
            }
            else {
                printf("ERROR: Path too long\n");
                return 1;
            }
        }

        printf("  Trying: %s ... ", tmpFullPath);

        // found it - return success
        if (file_exists(tmpFullPath, 0, NULL) != 0)
        {
            printf("FOUND!\n");
            // Copy the base path (without the ROM filename) to ROMPathFound
            if (Terminator < sizeof(ROMPathFound)) {
                memcpy(ROMPathFound, &ROMPath[Pos], Terminator);
                ROMPathFound[Terminator] = 0;
                printf("ROMPathFound set to: %s\n", ROMPathFound);
            }
            else {
                printf("ERROR: ROMPathFound too small for path segment\n");
            }
            printf("=========================\n\n");
            return 0;
        }
        else
        {
            printf("NOT FOUND\n");
        }

        // Move to next segment
        Pos += Terminator;
        if (ROMPath[Pos] == ';') {
            Pos++; // Skip the semicolon
        }

        // Check if we've reached the end of the ROMPath string
        if (Pos >= strlen(ROMPath)) {
            printf("Reached end of ROMPath string\n");
            break;
        }
    }

    printf("End of ROMPath reached - ROM not found!\n");
    printf("=========================\n\n");
    return 1;
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

void debug_current_palette(void) {
    printf("\n=== CURRENT PALETTE DEBUG ===\n");

    for (int i = 0; i < MAX_COL_PLANES; i++) {
        if (NumColPalettes[i] > 0) {
            printf("Palette for %d-bit (%d colors):\n", i, 1 << i);
            for (int j = 0; j < NumColPalettes[i]; j++) {
                printf("  Palette %d:\n", j);
                for (int k = 0; k < (1 << i); k++) {
                    RGB* color = &ColPalettes[i][j * (1 << i) + k];
                    printf("    Color %d: R=%d, G=%d, B=%d\n",
                        k, color->r, color->g, color->b);
                }
            }
        }
    }

    printf("=== END CURRENT PALETTE DEBUG ===\n\n");
}

void debug_all_palettes(void) {
    printf("\n=== ALL PALETTES DEBUG ===\n");

    for (int i = 0; i < MAX_COL_PLANES; i++) {
        if (NumColPalettes[i] > 0) {
            printf("Palette index %d (%d colors, %d palettes):\n",
                i, 1 << i, NumColPalettes[i]);

            for (int j = 0; j < NumColPalettes[i]; j++) {
                printf("  Palette %d:\n", j);
                for (int k = 0; k < (1 << i); k++) {
                    int idx = j * (1 << i) + k;
                    RGB color = ColPalettes[i][idx];
                    printf("    Color %d: R=%d, G=%d, B=%d\n",
                        k, color.r, color.g, color.b);
                }
            }
        }
    }

    printf("=== END ALL PALETTES DEBUG ===\n\n");
}

void fix_default_palettes(void) {
    printf("\n=== FIXING DEFAULT PALETTES ===\n");

    // Fix the 2-color palette (1 plane) to be black and white instead of black and red
    if (NumColPalettes[1] > 0 && ColPalettes[1] != NULL) {
        printf("Fixing 2-color palette from red to white\n");
        ColPalettes[1][0].r = 0;   // Black
        ColPalettes[1][0].g = 0;
        ColPalettes[1][0].b = 0;
        ColPalettes[1][1].r = 63;  // White (was red)
        ColPalettes[1][1].g = 63;
        ColPalettes[1][1].b = 63;

        printf("Fixed 2-color palette:\n");
        printf("  Color 0: R=%d, G=%d, B=%d\n",
            ColPalettes[1][0].r, ColPalettes[1][0].g, ColPalettes[1][0].b);
        printf("  Color 1: R=%d, G=%d, B=%d\n",
            ColPalettes[1][1].r, ColPalettes[1][1].g, ColPalettes[1][1].b);
    }

    printf("=== END FIXING DEFAULT PALETTES ===\n\n");
}

void fix_color_palettes(void) {
    printf("\n=== FIXING COLOR PALETTES ===\n");

    // Fix the 4-color palette (2 planes) to use actual colors
    if (NumColPalettes[2] > 0 && ColPalettes[2] != NULL) {
        printf("Fixing 4-color palette from grayscale to colors\n");

        // Color 0: Black (keep as is)
        ColPalettes[2][0].r = 0;
        ColPalettes[2][0].g = 0;
        ColPalettes[2][0].b = 0;

        // Color 1: Red
        ColPalettes[2][1].r = 63;
        ColPalettes[2][1].g = 0;
        ColPalettes[2][1].b = 0;

        // Color 2: Green
        ColPalettes[2][2].r = 0;
        ColPalettes[2][2].g = 63;
        ColPalettes[2][2].b = 0;

        // Color 3: Blue
        ColPalettes[2][3].r = 0;
        ColPalettes[2][3].g = 0;
        ColPalettes[2][3].b = 63;

        printf("Fixed 4-color palette:\n");
        for (int i = 0; i < 4; i++) {
            printf("  Color %d: R=%d, G=%d, B=%d\n",
                i, ColPalettes[2][i].r, ColPalettes[2][i].g, ColPalettes[2][i].b);
        }
    }

    printf("=== END FIXING COLOR PALETTES ===\n\n");
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

    printf("=== DEBUG ReadFromDriverIniFile: Starting ===\n");

    pBuffer = get_config_string("General", "Description", "None");

    // SAFE COMPARISON
    int has_valid_description = 0;
    if (pBuffer != NULL && pBuffer != (char*)(intptr_t)NULL) {
        printf("DEBUG: Description = '%s'\n", pBuffer);
        if (strcmp(pBuffer, "None") != 0) {
            has_valid_description = 1;
        }
    }

    if (!has_valid_description)
    {
        printf("ERROR: No valid description found\n");
        DisplayError("Can't load .ini driver!");
        if (pBuffer != NULL && pBuffer != (char*)(intptr_t)NULL) {
            SAFE_FREE(pBuffer);
        }
        return FALSE;
    }

    strncpy(GameDescription, pBuffer, MAX_GAME_DESCRIPTION);
    if (pBuffer != NULL && pBuffer != (char*)(intptr_t)NULL) {
        SAFE_FREE(pBuffer);
    }
    GameDescription[MAX_GAME_DESCRIPTION - 1] = 0;

    printf("DEBUG: GameDescription = '%s'\n", GameDescription);

    if ((NumGfxBanks = get_config_int("Layout", "GfxDecodes", 0)) == 0)
    {
        printf("ERROR: GfxDecodes is 0 or not found\n");
        DisplayError("INI file error 'GfxDecodes'");
        return FALSE;
    }

    printf("DEBUG: NumGfxBanks = %d\n", NumGfxBanks);

    // if this fail the orientation will be 0 - which is OK.
    Orientation = get_config_int("Layout", "Orientation", 0);
    printf("DEBUG: Orientation = %d\n", Orientation);

    GfxBanks = SAFE_MALLOC(NumGfxBanks * sizeof(SPRITE_PALETTE));
    if (GfxBanks == NULL)
    {
        printf("ERROR: Failed to allocate GfxBanks\n");
        DisplayError("GfxBanks malloc failed");
        return FALSE;
    }
    printf("DEBUG: GfxBanks allocated successfully\n");

    GfxBankExtraInfo = SAFE_MALLOC(NumGfxBanks * sizeof(GFXBANKEXTRA));
    if (GfxBankExtraInfo == NULL)
    {
        printf("ERROR: Failed to allocate GfxBankExtraInfo\n");
        SAFE_FREE(GfxBanks);
        DisplayError("GfxBanksExtra malloc failed");
        return FALSE;
    }
    printf("DEBUG: GfxBankExtraInfo allocated successfully\n");

    for (i = 0; i < NumGfxBanks; i++)
    {
        char GfxBank[10];
        int xoffset_count, yoffset_count;

        sprintf(GfxBank, "Decode%d", i + 1);
        printf("DEBUG: Processing %s\n", GfxBank);

        // read straight forward stuff from INI file
        GfxBanks[i].sprite_w = get_config_int(GfxBank, "width", -1);
        GfxBanks[i].sprite_h = get_config_int(GfxBank, "height", -1);
        GfxBanks[i].n_total = get_config_int(GfxBank, "total", -1);
        GfxBanks[i].first_sprite = 0; // set the position to the default
        GfxBankExtraInfo[i].planes = get_config_int(GfxBank, "planes", -1);
        GfxBankExtraInfo[i].startaddress = get_config_int(GfxBank, "start", -1);
        GfxBankExtraInfo[i].charincrement = get_config_int(GfxBank, "charincrement", -1);

        printf("DEBUG: %s values - w:%d, h:%d, total:%d, planes:%d, start:%d, charinc:%d\n",
            GfxBank, GfxBanks[i].sprite_w, GfxBanks[i].sprite_h, GfxBanks[i].n_total,
            GfxBankExtraInfo[i].planes, GfxBankExtraInfo[i].startaddress,
            GfxBankExtraInfo[i].charincrement);

        // did any fail?
        if ((GfxBanks[i].sprite_w == -1) || (GfxBanks[i].sprite_h == -1) ||
            (GfxBanks[i].n_total == -1) || (GfxBankExtraInfo[i].planes == -1) ||
            (GfxBankExtraInfo[i].startaddress == -1) || (GfxBankExtraInfo[i].charincrement == -1))
        {
            printf("ERROR: Missing required values in %s\n", GfxBank);
            SAFE_FREE(GfxBanks);
            SAFE_FREE(GfxBankExtraInfo);
            DisplayError("INI file corrupt");
            return FALSE;
        }

        // read plane offsets and store in array
        printf("DEBUG: Reading planeoffsets for %s\n", GfxBank);
        argv = get_config_argv(GfxBank, "planeoffsets", &argc);
        printf("DEBUG: planeoffsets argc = %d, expected %d\n", argc, GfxBankExtraInfo[i].planes);

        // sanity check
        if (argc != GfxBankExtraInfo[i].planes)
        {
            printf("ERROR: planeoffsets count mismatch: expected %d, got %d\n",
                GfxBankExtraInfo[i].planes, argc);
            SAFE_FREE(GfxBanks);
            SAFE_FREE(GfxBankExtraInfo);

            // Free the argv array if it was allocated
            if (argv) {
                for (k = 0; k < argc; k++) {
                    if (argv[k]) SAFE_FREE(argv[k]);
                }
                SAFE_FREE(argv);
            }

            DisplayError("INI file corrupt - planeoffsets");
            return FALSE;
        }

        // copy planeoffsets into array
        for (k = 0; k < argc; k++) {
            GfxBankExtraInfo[i].planeoffsets[k] = atol(argv[k]);
            printf("DEBUG: planeoffsets[%d] = %ld\n", k, GfxBankExtraInfo[i].planeoffsets[k]);
        }

        // Free the argv array for planeoffsets
        for (k = 0; k < argc; k++) {
            if (argv[k]) SAFE_FREE(argv[k]);
        }
        SAFE_FREE(argv);

        // read xoffsets and store in array
        printf("DEBUG: Reading xoffsets for %s\n", GfxBank);
        xoffset_count = get_config_int_array(GfxBank, "xoffsets", GfxBankExtraInfo[i].xoffset, 64);
        printf("DEBUG: xoffset_count = %d, expected %d\n", xoffset_count, GfxBanks[i].sprite_w);

        // sanity check
        if (xoffset_count != GfxBanks[i].sprite_w)
        {
            printf("ERROR: xoffsets count mismatch: expected %d, got %d\n",
                GfxBanks[i].sprite_w, xoffset_count);
            SAFE_FREE(GfxBanks);
            SAFE_FREE(GfxBankExtraInfo);
            DisplayError("INI file corrupt - xoffsets");
            return FALSE;
        }

        // read yoffsets and store in array
        printf("DEBUG: Reading yoffsets for %s\n", GfxBank);
        yoffset_count = get_config_int_array(GfxBank, "yoffsets", GfxBankExtraInfo[i].yoffset, 64);
        printf("DEBUG: yoffset_count = %d, expected %d\n", yoffset_count, GfxBanks[i].sprite_h);

        // sanity check
        if (yoffset_count != GfxBanks[i].sprite_h)
        {
            printf("ERROR: yoffsets count mismatch: expected %d, got %d\n",
                GfxBanks[i].sprite_h, yoffset_count);
            SAFE_FREE(GfxBanks);
            SAFE_FREE(GfxBankExtraInfo);
            DisplayError("INI file corrupt - yoffsets");
            return FALSE;
        }

        printf("DEBUG: %s processed successfully\n", GfxBank);
    }

    // Calculate the number of graphics roms
    printf("DEBUG: Counting GraphicsRoms...\n");
    NumGfxRoms = 0;
    int max_roms_to_check = 20;
    int rom_index;

    for (rom_index = 1; rom_index <= max_roms_to_check; rom_index++) {
        sprintf(Buffer, "Rom%d", rom_index);
        printf("DEBUG: Looking for [GraphicsRoms] %s\n", Buffer);
        char* rom_value = get_config_string("GraphicsRoms", Buffer, "None");

        if (rom_value != NULL) {
            printf("DEBUG: Got value: '%s'\n", rom_value);

            if (strcmp(rom_value, "None") != 0) {
                int rom_argc;
                char** rom_argv = get_config_argv("GraphicsRoms", Buffer, &rom_argc);

                printf("DEBUG: Rom%d parsed into %d arguments\n", rom_index, rom_argc);

                if (rom_argv && rom_argc == 3) {
                    NumGfxRoms++;
                    printf("DEBUG: Valid Rom%d found\n", rom_index);
                }
                else {
                    printf("WARNING: Invalid Rom%d - expected 3 values, got %d\n",
                        rom_index, rom_argc);
                }

                if (rom_argv) {
                    for (k = 0; k < rom_argc; k++) {
                        if (rom_argv[k]) SAFE_FREE(rom_argv[k]);
                    }
                    SAFE_FREE(rom_argv);
                }
            }
            else {
                printf("DEBUG: No more Roms found at Rom%d\n", rom_index);
                SAFE_FREE(rom_value);
                break;
            }

            SAFE_FREE(rom_value);
        }
        else {
            printf("DEBUG: get_config_string returned NULL for Rom%d\n", rom_index);
            break;
        }
    }

    printf("DEBUG: Found %d valid GraphicsRoms\n", NumGfxRoms);

    if (NumGfxRoms == 0)
    {
        printf("ERROR: No valid GraphicsRoms found!\n");
        SAFE_FREE(GfxBanks);
        SAFE_FREE(GfxBankExtraInfo);
        DisplayError("INI file corrupt - graphicsroms");
        return FALSE;
    }

    // Allocate memory for ROM structures
    GfxRoms = SAFE_MALLOC(NumGfxRoms * sizeof(GFXROM));
    if (GfxRoms == NULL)
    {
        printf("ERROR: Failed to allocate GfxRoms\n");
        SAFE_FREE(GfxBanks);
        SAFE_FREE(GfxBankExtraInfo);
        DisplayError("GfxRoms malloc failed");
        return FALSE;
    }
    printf("DEBUG: GfxRoms allocated successfully\n");

    // Read ROM information
    for (i = 0; i < NumGfxRoms; i++) {
        sprintf(Buffer, "Rom%d", i + 1);
        printf("DEBUG: Reading ROM info for %s\n", Buffer);
        argv = get_config_argv("GraphicsRoms", Buffer, &argc);

        if (argc != 3) {
            printf("ERROR: %s should have 3 arguments, got %d\n", Buffer, argc);
            // ... error handling
        }

        // Copy ROM info - FIXED: Use atol instead of atoi for large numbers
        GfxRoms[i].LoadAddress = atol(argv[0]);
        GfxRoms[i].Size = atol(argv[1]);  // This should now be 2048, 4096, etc.
        GfxRoms[i].Alternate = FALSE;

        if ((GfxRoms[i].Size & ALTERNATE_ROMS_DETECT) == ALTERNATE_ROMS_DETECT) {
            GfxRoms[i].Size &= ALTERNATE_ROMS_SIZE;
            GfxRoms[i].Alternate = TRUE;
        }

        strncpy(GfxRoms[i].ROMName, argv[2], MAX_ROM_NAME);
        GfxRoms[i].ROMName[MAX_ROM_NAME - 1] = 0;

        printf("DEBUG: ROM %d - Load:0x%lX, Size:%ld, Alt:%d, Name:'%s'\n",
            i, GfxRoms[i].LoadAddress, GfxRoms[i].Size,
            GfxRoms[i].Alternate, GfxRoms[i].ROMName);

        // Free argv array
        for (k = 0; k < argc; k++) {
            if (argv[k]) SAFE_FREE(argv[k]);
        }
        SAFE_FREE(argv);
    }

    // Debug ROM loading
    debug_rom_loading();

    // Read color palettes
    printf("DEBUG: Reading color palettes\n");
    if (ReadColourPalettes() == FALSE)
    {
        printf("ERROR: Failed to read color palettes\n");
        for (i = 0; i < MAX_COL_PLANES; i++)
        {
            if (ColPalettes[i] != NULL)
            {
                SAFE_FREE(ColPalettes[i]);
                ColPalettes[i] = NULL;
                NumColPalettes[i] = 0;
            }
        }
        SAFE_FREE(GfxBanks);
        SAFE_FREE(GfxBankExtraInfo);
        SAFE_FREE(GfxRoms);
        return FALSE;
    }

    fix_default_palettes();
    fix_color_palettes();

    debug_current_palette();
    debug_all_palettes();

    printf("DEBUG: ReadFromDriverIniFile completed successfully\n");
    return TRUE;
}


BOOL ReadColourPalettes(void) {
    int i, j;
    int palette_count;
    int PaletteVals[200];
    BOOL DefaultPaletteUsed[MAX_COL_PLANES];

    printf("=== DEBUG ReadColourPalettes: Starting ===\n");

    for (i = 0; i < MAX_COL_PLANES; i++) {
        DefaultPaletteUsed[i] = FALSE;
    }

    // define defaults to any colour planes used - these will
    // be overwritten if any exist in the ini file
    for (i = 0; i < NumGfxBanks; i++) {
        int BankPlanes = GfxBankExtraInfo[i].planes;
        if (NumColPalettes[BankPlanes] == 0) {
            int NumColours = 1 << BankPlanes;
            ColPalettes[BankPlanes] = SAFE_MALLOC(sizeof(RGB) * NumColours);
            if (ColPalettes[BankPlanes] == NULL) {
                DisplayError("Malloc failed creating default palettes");
                return FALSE;
            }
            memcpy(ColPalettes[BankPlanes], DefaultColours, NumColours * sizeof(RGB));
            NumColPalettes[BankPlanes] = 1;
            DefaultPaletteUsed[BankPlanes] = TRUE;
        }
    }

    // read in all of the palettes that exist in the ini file
    i = 0;
    for (;;) {
        char PaletteNum[12];
        int NumCols = 0;
        int Index;

        sprintf(PaletteNum, "Palette%d", i + 1);
        i++;

        printf("DEBUG: Looking for palette entry: %s\n", PaletteNum);

        // Get palette values from config
        palette_count = get_config_int_array("Palette", PaletteNum, PaletteVals, 200);
        printf("DEBUG: get_config_int_array returned %d values for %s\n", palette_count, PaletteNum);

        // palette didn't exist
        if (palette_count == 0) {
            printf("DEBUG: No more palettes found\n");
            break;
        }

        printf("DEBUG: Processing palette with %d values\n", palette_count);

        // Print all values for debugging
        for (int k = 0; k < palette_count; k++) {
            printf("DEBUG: PaletteVals[%d] = %d\n", k, PaletteVals[k]);
        }

        // sanity checking
        if (palette_count < 1) {
            printf("ERROR: Palette has no values\n");
            return FALSE;
        }

        NumCols = PaletteVals[0];
        Index = IndexFromNumColours(NumCols);

        printf("DEBUG: NumCols = %d, Index = %d\n", NumCols, Index);

        // Check if index is valid
        if (Index < 0 || Index >= MAX_COL_PLANES) {
            printf("ERROR: Invalid palette index %d for %d colors\n", Index, NumCols);
            return FALSE;
        }

        // do we have default for this colour plane?
        if (NumColPalettes[Index] == 0) {
            // no default therefore it doesn't belong to this graphics set
            // For now, we'll create one and continue
            int NumColours = 1 << Index;
            ColPalettes[Index] = SAFE_MALLOC(sizeof(RGB) * NumColours);
            if (ColPalettes[Index] == NULL) {
                printf("ERROR: Failed to allocate palette for index %d\n", Index);
                return FALSE;
            }
            memcpy(ColPalettes[Index], DefaultColours, NumColours * sizeof(RGB));
            NumColPalettes[Index] = 1;
            DefaultPaletteUsed[Index] = TRUE;
        }

        // are there the right number of colours?
        if (palette_count < 1 + NumCols * 3) {
            printf("ERROR: Not enough palette values: expected %d, got %d\n",
                1 + NumCols * 3, palette_count);
            return FALSE;
        }

        if (DefaultPaletteUsed[Index] == TRUE) {
            // we have already malloced a default palette here so overwrite it
            printf("DEBUG: Overwriting default palette for index %d\n", Index);
            // ensure we don't try and overwrite it again
            DefaultPaletteUsed[Index] = FALSE;
        }
        else {
            // add a new colour palette
            int PalSize = NumCols * sizeof(RGB);
            printf("DEBUG: Adding new palette for index %d (current count: %d)\n",
                Index, NumColPalettes[Index]);

            // create memory for larger palette
            RGB* tPal = SAFE_MALLOC(PalSize * (NumColPalettes[Index] + 1));
            if (tPal == NULL) {
                printf("ERROR: Failed to allocate expanded palette\n");
                DisplayError("Malloc failed creating palette");
                return FALSE;
            }

            // copy old palette to new memory
            memcpy(tPal, ColPalettes[Index], PalSize * NumColPalettes[Index]);
            NumColPalettes[Index]++;

            // free old memory and copy pointer to new palette
            SAFE_FREE(ColPalettes[Index]);
            ColPalettes[Index] = tPal;
        }

        // Copy palette values - FIXED indexing
        for (j = 0; j < NumCols; j++) {
            int Offset = NumCols * (NumColPalettes[Index] - 1);
            ColPalettes[Index][j + Offset].r = PaletteVals[1 + (j * 3)];     // Fixed: start from index 1
            ColPalettes[Index][j + Offset].g = PaletteVals[1 + (j * 3) + 1]; // Fixed: +1
            ColPalettes[Index][j + Offset].b = PaletteVals[1 + (j * 3) + 2]; // Fixed: +2

            printf("DEBUG: Palette[%d][%d] = RGB(%d, %d, %d)\n",
                Index, j + Offset,
                ColPalettes[Index][j + Offset].r,
                ColPalettes[Index][j + Offset].g,
                ColPalettes[Index][j + Offset].b);
        }

        printf("DEBUG: Successfully processed palette %s\n", PaletteNum);
    }

    printf("DEBUG: ReadColourPalettes completed successfully\n");
    return TRUE;
}


// to calculate this return the number of right shifts required before
// the parameter = 1
int IndexFromNumColours(int Cols)
{
    int Count = 0;
    int OriginalCols = Cols;

    printf("DEBUG IndexFromNumColours: Input = %d\n", Cols);

    if (Cols <= 0) {
        printf("ERROR: Invalid color count: %d\n", Cols);
        return -1;
    }

    while (Cols != 1)
    {
        if (Cols % 2 != 0) {
            printf("ERROR: Color count %d is not a power of 2\n", OriginalCols);
            return -1;
        }
        Cols = Cols >> 1;
        Count++;
    }

    printf("DEBUG IndexFromNumColours: %d colors -> %d bits (index %d)\n",
        OriginalCols, Count, Count);
    return Count;
}

// Helper function to extract directory from full path
void extract_directory_from_path(const char* full_path, char* dir_buffer, int buffer_size) {
    char* last_slash = strrchr(full_path, '/');
    if (!last_slash) last_slash = strrchr(full_path, '\\');

    if (last_slash) {
        int dir_len = last_slash - full_path;
        if (dir_len < buffer_size) {
            safe_strncpy(dir_buffer, full_path, dir_len);
            dir_buffer[dir_len] = '\0';
        }
        else {
            safe_strncpy(dir_buffer, full_path, buffer_size - 1);
            dir_buffer[buffer_size - 1] = '\0';
        }
    }
    else {
        // No directory in path
        dir_buffer[0] = '\0';
    }
}

BOOL LoadGfxRomData(void)
{
    int i;
    FILE* fp;
    char ROMDirWithName[512];
  //  char full_rom_path[512];
    long total = 0;
    long max_address = 0;
    long bank_requirements = 0;

    printf("\n=== LoadGfxRomData DEBUG ===\n");
    printf("ROMDirName: %s\n", ROMDirName);
    printf("First ROM: %s\n", GfxRoms[0].ROMName);
    printf("ROMPath: %s\n", ROMPath);

    // Enhanced ROM size validation
    for (i = 0; i < NumGfxRoms; i++) {
        printf("DEBUG: ROM%d - Name: %s, LoadAddress: 0x%lX, Size: %ld\n",
            i + 1, GfxRoms[i].ROMName, GfxRoms[i].LoadAddress, GfxRoms[i].Size);

        // Verify sizes match file sizes
        char FullPath[512];
        safe_strncpy(FullPath, ROMPathFound, sizeof(FullPath) - 1);
        put_backslash(FullPath, sizeof(FullPath));
        strcat(FullPath, ROMDirName);
        put_backslash(FullPath, sizeof(FullPath));
        strcat(FullPath, GfxRoms[i].ROMName);

        long file_size_check = file_size(FullPath);
        printf("DEBUG: File %s size: %ld bytes, expected: %ld bytes\n",
            GfxRoms[i].ROMName, file_size_check, GfxRoms[i].Size);

        if (file_size_check != GfxRoms[i].Size) {
            printf("WARNING: File size mismatch for %s\n", GfxRoms[i].ROMName);
        }
    }

    // Build the path to look for: ROMDirName + ROM filename
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

    // Calculate total memory needed - use the highest address + size from ROMs
    for (i = 0; i < NumGfxRoms; i++) {
        long end_address = GfxRoms[i].LoadAddress + GfxRoms[i].Size;
        if (end_address > max_address) {
            max_address = end_address;
        }
        printf("DEBUG: ROM%d: LoadAddress=0x%lX, Size=%ld, EndAddress=0x%lX\n",
            i + 1, GfxRoms[i].LoadAddress, GfxRoms[i].Size, end_address);
    }

    // Calculate memory needed for graphics banks
    for (i = 0; i < NumGfxBanks; i++) {
        long bank_end = GfxBankExtraInfo[i].startaddress +
            (GfxBankExtraInfo[i].charincrement * GfxBanks[i].n_total);
        if (bank_end > bank_requirements) {
            bank_requirements = bank_end;
        }
        printf("DEBUG: Bank %d: Start=0x%lX, CharIncrement=%d, TotalSprites=%d, EndAddress=0x%lX\n",
            i, GfxBankExtraInfo[i].startaddress, GfxBankExtraInfo[i].charincrement,
            GfxBanks[i].n_total, bank_end);
    }

    // Use the maximum of ROM requirements and bank requirements, plus safety margin
    total = (max_address > bank_requirements ? max_address : bank_requirements) + 1024;

    printf("DEBUG: Memory allocation calculation:\n");
    printf("  Max ROM address: 0x%lX (%ld bytes)\n", max_address, max_address);
    printf("  Max bank requirements: 0x%lX (%ld bytes)\n", bank_requirements, bank_requirements);
    printf("  Allocating: 0x%lX (%ld bytes) including safety margin\n", total, total);

    // Allocate memory for ROM data
    GfxRomData = SAFE_MALLOC(total);
    if (GfxRomData == NULL)
    {
        DisplayError("GfxRomData malloc failed for %ld bytes", total);
        return FALSE;
    }

    // Initialize memory to a known pattern (0xFF) to detect uninitialized reads
    memset(GfxRomData, 0xFF, total);

    // Load each ROM
    for (i = 0; i < NumGfxRoms; i++)
    {
        char FullPath[512];

        // Build full path
        safe_strncpy(FullPath, ROMPathFound, sizeof(FullPath) - 1);
        put_backslash(FullPath, sizeof(FullPath));
        strcat(FullPath, ROMDirName);
        put_backslash(FullPath, sizeof(FullPath));
        strcat(FullPath, GfxRoms[i].ROMName);

        printf("Trying to open ROM %d: %s\n", i + 1, FullPath);

        // Get file size first to verify
        long file_size_check = file_size(FullPath);
        printf("DEBUG: File size reported as: %ld bytes\n", file_size_check);

        if ((fp = fopen(FullPath, "rb")) == NULL)
        {
            SAFE_FREE(GfxRomData);
            DisplayError("File not found: %s", FullPath);
            printf("ERROR: Could not open: %s\n", FullPath);
            return FALSE;
        }

        printf("Successfully opened: %s\n", FullPath);

        if (GfxRoms[i].Alternate == FALSE)
        {
            size_t bytes_read = fread(&GfxRomData[GfxRoms[i].LoadAddress], 1, GfxRoms[i].Size, fp);
            printf("DEBUG: Read %zu bytes into address 0x%lX\n", bytes_read, GfxRoms[i].LoadAddress);

            if (bytes_read != GfxRoms[i].Size)
            {
                SAFE_FREE(GfxRomData);
                DisplayError("ROM Size load error: expected %ld, got %zu", GfxRoms[i].Size, bytes_read);
                fclose(fp);
                return FALSE;
            }
        }
        else
        {
            printf("DEBUG: Loading in alternate mode\n");
            long t;
            for (t = 0; t < GfxRoms[i].Size; t++)
            {
                long Address = GfxRoms[i].LoadAddress + (t * 2);
                int ch = fgetc(fp);
                if (ch == EOF) break;
                GfxRomData[Address] = ch;
            }
        }

        fclose(fp);
    }

    // After loading all ROMs, verify the first bank data
    printf("\n=== FIRST BANK DATA VERIFICATION ===\n");
    printf("First bank start address: 0x0\n");
    printf("First 32 bytes of ROM data at address 0x0:\n");
    for (int i = 0; i < 32 && i < total; i++) {
        printf("%02X ", GfxRomData[i] & 0xFF);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("=== END FIRST BANK VERIFICATION ===\n\n");

    return TRUE;
}


BOOL AllocateGfxBanks(void)
{
    int i, j;

    // create the bitmaps inorder to store the graphics banks
    for (i = 0; i < NumGfxBanks; i++)
    {
        GfxBanks[i].bmp = create_bitmap(GfxBanks[i].sprite_w * GfxBanks[i].n_total, GfxBanks[i].sprite_h);
        if (GfxBanks[i].bmp == NULL)
        {
            for (j = 0; j < NumGfxBanks - 1; j++)
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

// Add this function to debug graphics decoding
void debug_graphics_decoding(void) {
    printf("\n=== GRAPHICS DECODING DEBUG ===\n");
    printf("Number of graphics banks: %d\n", NumGfxBanks);

    for (int i = 0; i < NumGfxBanks; i++) {
        printf("Bank %d:\n", i);
        printf("  Dimensions: %dx%d\n", GfxBanks[i].sprite_w, GfxBanks[i].sprite_h);
        printf("  Total sprites: %d\n", GfxBanks[i].n_total);
        printf("  Start address: 0x%lX\n", GfxBankExtraInfo[i].startaddress);
        printf("  Planes: %d\n", GfxBankExtraInfo[i].planes);
        printf("  Char increment: %d\n", GfxBankExtraInfo[i].charincrement);

        // Calculate expected data size
        int bytes_per_sprite = GfxBanks[i].sprite_w * GfxBanks[i].sprite_h * GfxBankExtraInfo[i].planes / 8;
        printf("  Expected bytes per sprite: %d\n", bytes_per_sprite);
        printf("  Total expected data: %d bytes\n", bytes_per_sprite * GfxBanks[i].n_total);

        // Check if we have enough ROM data
        long end_address = GfxBankExtraInfo[i].startaddress + (bytes_per_sprite * GfxBanks[i].n_total);
        printf("  Data range: 0x%lX to 0x%lX\n", GfxBankExtraInfo[i].startaddress, end_address);

        if (end_address > 24576) { // Total ROM size
            printf("  WARNING: Bank extends beyond loaded ROM data!\n");
        }
    }
    printf("=== END GRAPHICS DECODING DEBUG ===\n\n");
}

BOOL LoadDriver(const char* INIFileName)
{
    BOOL retval;
    char INIDriverName[256];
    char* fname = get_filename(INIFileName);
    char driver_dir[256] = "";
    int i = 0;

    printf("=== DEBUG LoadDriver: Starting ===\n");
    printf("DEBUG: INI file: %s\n", INIFileName);

    // free any currently loaded driver
    FreeDriver();

    /* setup INI Driver name */
    sprintf(INIDriverName, "%s", INIFileName);

    /* Extract game name from filename for ROMDirName */
    i = 0;
    while ((fname[i] != '\0') && (fname[i] != '.')) {
        ROMDirName[i] = fname[i];
        i++;
    }
    ROMDirName[i] = 0;

    printf("DEBUG: ROMDirName = '%s'\n", ROMDirName);

    // load the data from the INI file
    push_config_state();
    set_config_file(INIFileName);
    sprintf(INI_Driver_Path, "%s", INIFileName);

    printf("DEBUG: Calling ReadFromDriverIniFile\n");
    retval = ReadFromDriverIniFile();
    pop_config_state();

    // did the INI read fail?
    if (retval == FALSE)
    {
        printf("ERROR: ReadFromDriverIniFile returned FALSE\n");
        return retval;
    }

    printf("DEBUG: INI file read successfully, loading ROM data\n");

    // load graphics roms into memory
    retval = LoadGfxRomData();

    // if load failed then free all INI Driver memory and fail
    if (retval == FALSE) {
        printf("ERROR: LoadGfxRomData returned FALSE\n");
        if (GfxBanks)         SAFE_FREE(GfxBanks);
        if (GfxBankExtraInfo) SAFE_FREE(GfxBankExtraInfo);
        if (GfxRoms)          SAFE_FREE(GfxRoms);
        return retval;
    }

    printf("DEBUG: ROM data loaded successfully\n");

    // After loading all ROMs
    printf("DEBUG: Verifying ROM loading:\n");
    for (i = 0; i < NumGfxRoms; i++) {
        printf("ROM %d (LoadAddress: 0x%lX): ", i, GfxRoms[i].LoadAddress);
        printf("First byte: 0x%02X, Last byte: 0x%02X\n",
            GfxRomData[GfxRoms[i].LoadAddress],
            GfxRomData[GfxRoms[i].LoadAddress + GfxRoms[i].Size - 1]);
    }

    // create bitmaps for graphics banks
    retval = AllocateGfxBanks();

    // if load failed then free all INI Driver memory and loaded roms and fail
    if (retval == FALSE) {
        printf("ERROR: AllocateGfxBanks returned FALSE\n");
        if (GfxBanks)         SAFE_FREE(GfxBanks);
        if (GfxBankExtraInfo) SAFE_FREE(GfxBankExtraInfo);
        if (GfxRoms)          SAFE_FREE(GfxRoms);
        if (GfxRomData)       SAFE_FREE(GfxRomData);
        return retval;
    }

    printf("DEBUG: Graphics banks allocated successfully\n");

    debug_graphics_decoding();

    // MARK DRIVER AS LOADED BEFORE SWITCHING BANKS
    GameDriverLoaded = TRUE;  // <-- MOVE THIS HERE

    // now decode the graphics banks
    printf("DEBUG: Switching graphics bank\n");
    SwitchGraphicsBank(-1, 0);

    printf("DEBUG: Driver loaded successfully\n");

    return retval;
}

void FreeDriver(void)
{
    // no need to free up if none loaded
    if (GameDriverLoaded == TRUE)
    {
        int i;

        // get rid of the bitmaps first
        for (i = 0; i < NumGfxBanks; i++)
            destroy_bitmap(GfxBanks[i].bmp);

        if (GfxBanks)         SAFE_FREE(GfxBanks);
        if (GfxBankExtraInfo) SAFE_FREE(GfxBankExtraInfo);
        if (GfxRoms)          SAFE_FREE(GfxRoms);
        if (GfxRomData)       SAFE_FREE(GfxRomData);  // Make sure this is freed!

        // free colour palettes
        for (i = 0; i < MAX_COL_PLANES; i++)
        {
            if (ColPalettes[i] != NULL)
            {
                SAFE_FREE(ColPalettes[i]);
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
        if (argv[i]) SAFE_FREE(argv[i]);
    }
    SAFE_FREE(argv);

    printf("get_config_int_array: parsed %d values\n", count);
    return count;
}


/* Copies a file. filename includes relative path */
/* */
BOOL CopyROM(char* from, char* to, long size, int CommandLine)
{
    FILE* fp_from;
    FILE* fp_to;
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


static void MakeBackupPath(const char* DirName, char* Buffer, char* Filename, int AppendFilename) {
    if (Buffer == NULL) return;

    size_t buffer_size = 512;

    if (AppendFilename && Filename != NULL) {
        // Build full path with filename
        snprintf(Buffer, buffer_size, "%s/%s/%s/%s",
            ROMPathFound, ROMDirName, DirName, Filename);
    }
    else {
        // Build path without filename
        snprintf(Buffer, buffer_size, "%s/%s/%s",
            ROMPathFound, ROMDirName, DirName);
    }

    // Replace forward slashes with backslashes if needed (Windows)
#ifdef _WIN32
    for (char* p = Buffer; *p; p++) {
        if (*p == '/') *p = '\\';
    }
#endif

    printf("DEBUG: MakeBackupPath result: '%s'\n", Buffer);
}




void VerifyEncoding(int bank_index) {
    printf("\n=== VERIFYING ENCODING FOR BANK %d ===\n", bank_index);

    if (bank_index < 0 || bank_index >= NumGfxBanks) {
        printf("ERROR: Invalid bank index %d\n", bank_index);
        return;
    }

    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[bank_index];
    SPRITE_PALETTE* sp = &GfxBanks[bank_index];

    printf("Bank %d: %dx%d sprites, %d total, start address: 0x%lX\n",
        bank_index, sp->sprite_w, sp->sprite_h, sp->n_total, gbe->startaddress);

    // Calculate expected data size
    int bytes_per_sprite = (sp->sprite_w * sp->sprite_h * gbe->planes) / 8;
    printf("Expected bytes per sprite: %d\n", bytes_per_sprite);
    printf("Total expected data: %d bytes\n", bytes_per_sprite * sp->n_total);

    // Check first sprite data
    long first_sprite_addr = gbe->startaddress;
    printf("First sprite address: 0x%lX\n", first_sprite_addr);

    printf("First 32 bytes of encoded data at address 0x%lX:\n", first_sprite_addr);
    for (int i = 0; i < 32 && (first_sprite_addr + i) < MAX_ROM_SIZE; i++) {
        printf("%02X ", GfxRomData[first_sprite_addr + i] & 0xFF);
        if ((i + 1) % 16 == 0) printf("\n");
    }

    // Compare with what we expect to see for the first sprite
    printf("\nChecking sprite 0 data in master bitmap:\n");
    MYBITMAP* master_bmp = sp->bmp;
    if (master_bmp) {
        printf("First row of sprite 0 in master bitmap:\n");
        for (int x = 0; x < sp->sprite_w && x < 8; x++) {
            int color = getpixel(master_bmp, x, 0);
            printf("  [%d,0] = %d\n", x, color);
        }
    }

    printf("=== END VERIFYING ENCODING ===\n\n");
}

/* To avoid original roms being overwritten a subdirectory called agebak */
/* is created and a copy of all of the original roms is store there. */
/* If agebak already exists then nothing is copied to avoid overwriting */
/* the back ups of the originals. */
BOOL MakeBackups(void)
{
    char BackupPath[512] = { 0 };  // Initialize to zeros
    char FromPath[512] = { 0 };
    FILE* fp;
    int i;

    printf("DEBUG MakeBackups:\n");
    printf("  ROMPathFound: '%s'\n", ROMPathFound ? ROMPathFound : "(NULL)");
    printf("  ROMDirName: '%s'\n", ROMDirName ? ROMDirName : "(NULL)");
    printf("  NumGfxRoms: %d\n", NumGfxRoms);

    if (NumGfxRoms == 0) {
        printf("ERROR: No ROMs loaded!\n");
        return FALSE;
    }

    if (GfxRoms[0].ROMName == NULL) {
        printf("ERROR: First ROM name is NULL!\n");
        return FALSE;
    }

    printf("  First ROM name: '%s'\n", GfxRoms[0].ROMName);

    MakeBackupPath(BackupDir, BackupPath, GfxRoms[0].ROMName, 1);

    /* are the roms already backed up? (can we open one for read) */
    if ((fp = fopen(BackupPath, "rb")) != NULL)
    {
        fclose(fp);
        printf("DEBUG: Backups already exist at: %s\n", BackupPath);
        return TRUE;
    }

    printf("DEBUG: Creating backups at: %s\n", BackupPath);

    /* at this point we need to make backups */

    /* first create directory */
    MakeBackupPath(BackupDir, BackupPath, GfxRoms[0].ROMName, 0);

    printf("DEBUG: Creating backup directory: %s\n", BackupPath);
    _mkdir(BackupPath);

    /* back up the roms */
    for (i = 0; i < NumGfxRoms; i++)
    {
        if (GfxRoms[i].ROMName == NULL) {
            printf("WARNING: ROM %d has NULL name, skipping\n", i);
            continue;
        }

        safe_strncpy(FromPath, ROMPathFound, sizeof(FromPath) - 1);
        put_backslash(FromPath, sizeof(FromPath));
        strncat(FromPath, ROMDirName, sizeof(FromPath) - strlen(FromPath) - 1);
        put_backslash(FromPath, sizeof(FromPath));
        strncat(FromPath, GfxRoms[i].ROMName, sizeof(FromPath) - strlen(FromPath) - 1);

        MakeBackupPath(BackupDir, BackupPath, GfxRoms[i].ROMName, 1);

        printf("DEBUG: Backing up %s to %s\n", FromPath, BackupPath);

        if (CopyROM(FromPath, BackupPath, GfxRoms[i].Size, 0) == FALSE) {
            printf("ERROR: Failed to backup ROM %d\n", i);
            return FALSE;
        }
    }

    printf("DEBUG: All ROMs backed up successfully\n");
    return TRUE;
}

void VerifyColorEncoding(int bank_index) {
    printf("\n=== FIXED BIT POSITION CALCULATION DEBUG Bank %d ===\n", bank_index);

    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[bank_index];
    SPRITE_PALETTE* sp = &GfxBanks[bank_index];

    long StartBit = gbe->startaddress * 8;
    printf("StartBit: %ld\n", StartBit);

    // Calculate expected bit positions for first sprite, first pixel
    long offs1 = (gbe->charincrement * 0) + StartBit;
    printf("offs1 (sprite0): %ld\n", offs1);

    for (int x = 0; x < 2 && x < sp->sprite_w; x++) {
        for (int y = 0; y < 2 && y < sp->sprite_h; y++) {
            long base_bitpos = offs1 + gbe->xoffset[x] + gbe->yoffset[y];
            printf("Pixel[%d,%d]: xoffset=%d, yoffset=%d, base_bitpos=%ld\n",
                x, y, gbe->xoffset[x], gbe->yoffset[y], base_bitpos);

            for (int plane = 0; plane < gbe->planes; plane++) {
                // FIXED: Use interleaved plane positioning
                long bit_position = base_bitpos + plane;
                printf("  Plane %d: bitpos=%ld (interleaved)\n", plane, bit_position);
            }
        }
    }

    printf("=== END FIXED BIT POSITION CALCULATION DEBUG ===\n\n");
}

void AnalyzeOriginalROM(void) {
    printf("\n=== ORIGINAL ROM DATA ANALYSIS ===\n");

    printf("First 32 bytes of ROM data (bank 0):\n");
    for (int i = 0; i < 32 && i < MAX_ROM_SIZE; i++) {
        printf("%02X ", GfxRomData[i] & 0xFF);
        if ((i + 1) % 16 == 0) printf("\n");
    }

    // Analyze the first sprite in the original ROM
    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[0];
    SPRITE_PALETTE* sp = &GfxBanks[0];

    long StartBit = gbe->startaddress * 8;
    long offs1 = (gbe->charincrement * 0) + StartBit;

    printf("\nFirst sprite (sprite 0) bit analysis:\n");
    printf("Start bit: %ld\n", offs1);

    // Manually decode the first few pixels to understand the bitplane layout
    for (int y = 0; y < 2; y++) {
        for (int x = 0; x < 4; x++) {
            long ww = offs1 + gbe->xoffset[x] + gbe->yoffset[y];
            printf("Pixel[%d,%d] at bit %ld: ", x, y, ww);

            // Check each plane
            for (int plane = 0; plane < gbe->planes; plane++) {
                long bitpos = ww + plane;
                int bit_value = GetBit(bitpos);
                printf("Plane%d=%d ", plane, bit_value);
            }

            // Calculate the color index
            int result = 0;
            for (int plane = 0; plane < gbe->planes; plane++) {
                long bitpos = ww + plane;
                if (GetBit(bitpos)) {
                    result |= (1 << (gbe->planes - 1 - plane));
                }
            }
            printf("-> Color index: %d\n", result);
        }
    }

    printf("=== END ROM ANALYSIS ===\n\n");
}

void TestBitplaneArrangements(int bank_index) {
    printf("\n=== BITPLANE ARRANGEMENT TEST Bank %d ===\n", bank_index);

    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[bank_index];

    // Test different bitplane arrangements
    printf("Testing color index 3 (should be blue):\n");
    int color_index = 3;

    // Arrangement 1: Current (MSB to LSB)
    printf("Arrangement 1 (MSB to LSB): ");
    for (int plane = 0; plane < gbe->planes; plane++) {
        int pln_mask = 1 << (gbe->planes - 1 - plane);
        int bit = (color_index & pln_mask) ? 1 : 0;
        printf("Plane%d=%d ", plane, bit);
    }
    printf("\n");

    // Arrangement 2: LSB to MSB
    printf("Arrangement 2 (LSB to MSB): ");
    for (int plane = 0; plane < gbe->planes; plane++) {
        int pln_mask = 1 << plane;
        int bit = (color_index & pln_mask) ? 1 : 0;
        printf("Plane%d=%d ", plane, bit);
    }
    printf("\n");

    // Arrangement 3: Reversed planes
    printf("Arrangement 3 (Reversed planes): ");
    for (int plane = 0; plane < gbe->planes; plane++) {
        int reversed_plane = gbe->planes - 1 - plane;
        int pln_mask = 1 << reversed_plane;
        int bit = (color_index & pln_mask) ? 1 : 0;
        printf("Plane%d=%d ", plane, bit);
    }
    printf("\n");

    printf("=== END BITPLANE TEST ===\n\n");
}

void VerifyActualBitPositions(int bank_index) {
    printf("\n=== ACTUAL BIT POSITIONS VERIFICATION Bank %d ===\n", bank_index);

    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[bank_index];
    SPRITE_PALETTE* sp = &GfxBanks[bank_index];

    long StartBit = gbe->startaddress * 8;
    long offs1 = (gbe->charincrement * 0) + StartBit;

    printf("Testing first sprite encoding:\n");

    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            long base_bitpos = offs1 + gbe->xoffset[x] + gbe->yoffset[y];
            printf("Pixel[%d,%d] -> Base: %ld, Planes: ", x, y, base_bitpos);

            for (int plane = 0; plane < gbe->planes; plane++) {
                printf("%ld ", base_bitpos + plane);
            }
            printf("\n");
        }
    }

    printf("=== END ACTUAL BIT POSITIONS VERIFICATION ===\n\n");
}

extern int currentGfxBank;

// Saves the romset out to disk. Attempts to backup originals if
// a backup doesn't already exist.
BOOL SaveDriver(void)
{
    FILE* fp = NULL;
    int i = 0;
    char FullPath[512] = { 0 };

    printf("DEBUG SaveDriver: Starting save process\n");
    VerifySaveIntegrity();

    /* Take checksum of ROM data before encoding */
    unsigned long checksum_before = 0;
    for (int j = 0; j < 100 && j < MAX_ROM_SIZE; j++) {
        checksum_before += GfxRomData[j];
    }
    printf("ROM data checksum before encoding: %lu\n", checksum_before);

    AnalyzeOriginalROM();

    
    // In SaveDriver, before encoding
    printf("DEBUG: Testing GfxRomData writeability\n");
    printf("GfxRomData pointer: %p\n", GfxRomData);

    // Test writing to different locations
    printf("Testing write to GfxRomData[0]:\n");
    unsigned char original_0 = GfxRomData[0];
    printf("  Before: 0x%02X\n", original_0);
    GfxRomData[0] = 0xAA;
    printf("  After: 0x%02X\n", GfxRomData[0]);
    GfxRomData[0] = original_0; // Restore

    printf("Testing write to GfxRomData[100]:\n");
    unsigned char original_100 = GfxRomData[100];
    printf("  Before: 0x%02X\n", original_100);
    GfxRomData[100] = 0xBB;
    printf("  After: 0x%02X\n", GfxRomData[100]);

    GfxRomData[100] = original_100; // Restore
    // In SaveDriver, before encoding
    printf("DEBUG: Bank 0 information:\n");
    printf("  Start address: 0x%lX\n", GfxBankExtraInfo[0].startaddress);
    printf("  Char increment: %d\n", GfxBankExtraInfo[0].charincrement);
    printf("  Total sprites: %d\n", GfxBanks[0].n_total);

    // Calculate the end address of bank 0
    long bank0_end = GfxBankExtraInfo[0].startaddress +
        (GfxBankExtraInfo[0].charincrement * GfxBanks[0].n_total);
    printf("  Bank 0 data range: 0x%lX to 0x%lX\n",
        GfxBankExtraInfo[0].startaddress, bank0_end);

    // We can estimate the total ROM size from the ROM definitions
    long estimated_total = 0;
    for (int i = 0; i < NumGfxRoms; i++) {
        long rom_end = GfxRoms[i].LoadAddress + GfxRoms[i].Size;
        if (rom_end > estimated_total) {
            estimated_total = rom_end;
        }
    }
    printf("  Estimated total ROM size: 0x%lX bytes\n", estimated_total);

    // Verify this range is within loaded ROM data
    if (bank0_end > estimated_total) {
        printf("ERROR: Bank 0 extends beyond estimated ROM data!\n");
    }

    // In SaveDriver, before encoding
    printf("DEBUG: Manual ROM modification test\n");

    // Save original values
    unsigned char orig_byte7 = GfxRomData[7];
    unsigned char orig_byte8 = GfxRomData[8];

    printf("Before modification:\n");
    printf("  GfxRomData[7] = 0x%02X\n", orig_byte7);
    printf("  GfxRomData[8] = 0x%02X\n", orig_byte8);

    // Modify bytes that should be in bank 0
    GfxRomData[7] = 0xAA;
    GfxRomData[8] = 0xBB;

    printf("After modification:\n");
    printf("  GfxRomData[7] = 0x%02X (should be 0xAA)\n", GfxRomData[7]);
    printf("  GfxRomData[8] = 0x%02X (should be 0xBB)\n", GfxRomData[8]);

    // Restore original values
    GfxRomData[7] = orig_byte7;
    GfxRomData[8] = orig_byte8;

    for (int i = 0; i < NumGfxBanks; i++) {
        TestBitplaneArrangements(i);
    }

    for (int i = 0; i < NumGfxBanks; i++) {
        VerifyActualBitPositions(i);
    }

    /* First, encode all modified graphics banks back to ROM data */
    printf("DEBUG: Encoding modified graphics banks to ROM data\n");

    for (int i = 0; i < NumGfxBanks; i++) {
        VerifyColorEncoding(i);
    }

    SwitchGraphicsBank(currentGfxBank, currentGfxBank);

    printf("Commit_Graphics_Bank: currentGfxBank = %d\n", currentGfxBank);

    /* Take checksum after encoding */
    unsigned long checksum_after = 0;
    for (int j = 0; j < 100 && j < MAX_ROM_SIZE; j++) {
        checksum_after += GfxRomData[j];
    }
    printf("ROM data checksum after encoding: %lu\n", checksum_after);
    printf("ROM data changed: %s\n", (checksum_before != checksum_after) ? "YES" : "NO");

    /* try and make the back ups */
    if (!MakeBackups()) {
        printf("ERROR: MakeBackups failed\n");
        return FALSE;
    }

    printf("=== PRE-ENCODING VERIFICATION ===\n");
    VerifyCurrentSpriteToMaster();

    /* Now save the ROM files */
    for (i = 0; i < NumGfxRoms; i++)
    {
        if (GfxRoms[i].ROMName == NULL) {
            printf("ERROR: ROM %d has NULL name, cannot save\n", i);
            continue;
        }

        safe_strncpy(FullPath, ROMPathFound, sizeof(FullPath) - 1);
        put_backslash(FullPath, sizeof(FullPath));
        strncat(FullPath, ROMDirName, sizeof(FullPath) - strlen(FullPath) - 1);
        put_backslash(FullPath, sizeof(FullPath));
        strncat(FullPath, GfxRoms[i].ROMName, sizeof(FullPath) - strlen(FullPath) - 1);

        printf("DEBUG: Saving ROM %d to: %s\n", i, FullPath);
        printf("DEBUG: LoadAddress: 0x%lX, Size: %ld\n",
            GfxRoms[i].LoadAddress, GfxRoms[i].Size);

        // Check file size before writing
        long file_size_before = file_size(FullPath);
        printf("DEBUG: File size before write: %ld bytes\n", file_size_before);

        if ((fp = fopen(FullPath, "wb")) == NULL)
        {
            printf("ERROR: Failed to open for write: %s\n", FullPath);
            DisplayError("Failed to open for write: %s", FullPath);
            return FALSE;
        }

        if (GfxRoms[i].Alternate == FALSE)
        {
            size_t written = fwrite(&GfxRomData[GfxRoms[i].LoadAddress], 1, GfxRoms[i].Size, fp);
            printf("DEBUG: Wrote %zu bytes to file\n", written);

            if (written != GfxRoms[i].Size)
            {
                printf("ERROR: Failed to save: wrote %zu of %ld bytes\n", written, GfxRoms[i].Size);
                DisplayError("Failed to save: wrote %zu of %ld bytes", written, GfxRoms[i].Size);
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
                fputc(GfxRomData[Address], fp);
            }
        }

        fclose(fp);
    }

    long file_size_after = file_size(FullPath);
    printf("DEBUG: File size after write: %ld bytes\n", file_size_after);
    printf("DEBUG: File write successful: %s\n", (file_size_after == GfxRoms[i].Size) ? "YES" : "NO");

    return TRUE;
}



void WritePatchChanges(long StartSeq, long SeqLen, FILE* fpModifiedRom, FILE* fpPatchRom)
{
    int i;

    /* write value of j as 24bit number */
    char* p = (char*)&StartSeq;
    fputc(p[2], fpPatchRom);
    fputc(p[1], fpPatchRom);
    fputc(p[0], fpPatchRom);

    /* write value of SeqLen as 16bit number */
    p = (char*)&SeqLen;
    fputc(p[1], fpPatchRom);
    fputc(p[0], fpPatchRom);

    /* rewind fpModifiedRom to start of change */
    fseek(fpModifiedRom, StartSeq, SEEK_SET);

    /* copy bytes */
    for (i = 0; i < SeqLen; i++)
    {
        BYTE ch = fgetc(fpModifiedRom);
        fputc(ch, fpPatchRom);
    }

    /* make fpModifiedRom in the same position as when we entered this function */
    fgetc(fpModifiedRom);
}


void DoComparison(char* ROMName, long size)
{
    char BackupPath[512] = { 0 };
    char PatchPath[512] = { 0 };
    char FullPath[512] = { 0 };
    FILE* fpModifiedRom = NULL;
    FILE* fpBackupRom = NULL;
    FILE* fpPatchRom = NULL;
    long StartSeq = -1, SeqLen = 0, j;
    int HeaderWritten = 0;

    printf("DEBUG DoComparison: Starting for ROM '%s', size=%ld\n", ROMName, size);

    // Safety checks
    if (ROMName == NULL) {
        printf("ERROR: ROMName is NULL in DoComparison\n");
        return;
    }
    if (ROMPathFound == NULL) {
        printf("ERROR: ROMPathFound is NULL in DoComparison\n");
        return;
    }
    if (ROMDirName == NULL) {
        printf("ERROR: ROMDirName is NULL in DoComparison\n");
        return;
    }

    printf("DEBUG: ROMPathFound='%s'\n", ROMPathFound);
    printf("DEBUG: ROMDirName='%s'\n", ROMDirName);
    
    /* open the backup ROM (do this first as it is the most likely to fail) */
    MakeBackupPath(BackupDir, BackupPath, ROMName, 1);
    printf("DEBUG: BackupPath='%s'\n", BackupPath);

    if ((fpBackupRom = fopen(BackupPath, "rb")) == NULL)
    {
        /* just be quiet about this as it only means that the user hasn't edited */
        /* either the maps or the graphics therefore there is no backup for them */
        /* and no patch needed anyway */
        printf("DEBUG: No backup found, skipping patch for %s\n", ROMName);
        return;
    }

    /* now open the modified rom */
    printf("DEBUG: Building FullPath for modified ROM...\n");

    // Use safer string operations instead of strncpy
    FullPath[0] = '\0'; // Initialize

    // Safely copy ROMPathFound
    size_t remaining = sizeof(FullPath) - 1;
    strncat(FullPath, ROMPathFound, remaining);
    remaining = sizeof(FullPath) - strlen(FullPath) - 1;

    put_backslash(FullPath, sizeof(FullPath));
    remaining = sizeof(FullPath) - strlen(FullPath) - 1;

    // Safely append ROMDirName
    if (remaining > 0) {
        strncat(FullPath, ROMDirName, remaining);
        remaining = sizeof(FullPath) - strlen(FullPath) - 1;
    }

    put_backslash(FullPath, sizeof(FullPath));
    remaining = sizeof(FullPath) - strlen(FullPath) - 1;

    // Safely append ROMName
    if (remaining > 0) {
        strncat(FullPath, ROMName, remaining);
    }

    printf("DEBUG: FullPath='%s'\n", FullPath);

    if ((fpModifiedRom = fopen(FullPath, "rb")) == NULL)
    {
        printf("ERROR: Failed to open modified rom \"%s\"\n skipping", ROMName);
        fclose(fpBackupRom);
        return;
    }

    /* on to the comparison and creating the IPS file */
    /* Compare Roms */
    StartSeq = -1;

    printf("DEBUG: Starting ROM comparison...\n");

    for (j = 0; j < size; j++)
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
                    MakeBackupPath(PatchDir, PatchPath, ROMName, 1);
                    printf("DEBUG: Creating patch file: %s\n", PatchPath);
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

            SeqLen++;
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
        printf("DEBUG: Patch file completed for %s\n", ROMName);
    }

    fclose(fpBackupRom);
    fclose(fpModifiedRom);

    printf("DEBUG: DoComparison completed for %s\n", ROMName);
}


/* Make IPS patch file for specified game and put them in the AGEPATCH */
/* directory. This is only called from Command line so we can use printf */
/* for errors */
void MakePatch(void)
{
    char PatchPath[512] = { 0 };  // Increased buffer size
    int i = 0;

    printf("=== DEBUG MakePatch: Starting patch generation ===\n");

    // Safety checks
    if (NumGfxRoms == 0) {
        printf("ERROR: No ROMs loaded in MakePatch\n");
        return;
    }
    if (GfxRoms[0].ROMName == NULL) {
        printf("ERROR: First ROM name is NULL in MakePatch\n");
        return;
    }

    printf("DEBUG: Creating patch directory...\n");

    /* create patch directory */
    MakeBackupPath(PatchDir, PatchPath, GfxRoms[0].ROMName, 0);
    printf("DEBUG: Patch directory path: %s\n", PatchPath);

    // Create the directory
    int result = _mkdir(PatchPath);
    if (result != 0) {
        // Check if directory already exists (error code EEXIST)
        if (errno != EEXIST) {
            printf("ERROR: Failed to create patch directory: %s, error: %d\n", PatchPath, errno);
            return;
        }
        else {
            printf("DEBUG: Patch directory already exists\n");
        }
    }
    else {
        printf("DEBUG: Patch directory created successfully\n");
    }

    /* first do graphics roms */
    printf("DEBUG: Processing %d ROMs for patching\n", NumGfxRoms);
    for (i = 0; i < NumGfxRoms; i++)
    {
        if (GfxRoms[i].ROMName == NULL) {
            printf("WARNING: ROM %d has NULL name, skipping\n", i);
            continue;
        }
        // build full path including filename
        printf("DEBUG: Generating patch for ROM %d: %s (size: %ld)\n",
            i, GfxRoms[i].ROMName, GfxRoms[i].Size);
        DoComparison(GfxRoms[i].ROMName, GfxRoms[i].Size);
    }

    printf("=== DEBUG MakePatch: Patch generation completed ===\n");

#if 0
    /* are there any map roms to create patch files for? */
    if (numDataRoms != 0)
    {
        for (i = 0; i < numDataRoms; i++)
        {
            DoComparison(DataRoms[i].ROMName, DataRoms[i].Size);
        }
    }
#endif
}

// saves the current palette
void SavePalette(int NumCols, int PalIndex, int CurrentPal, int IniFilePalNum)
{
    char* Buffer;
    char NumBuf[10];
    int j;

    // This buffer will hold number like the following:
    // 4 255 255 255  255 255 255
    // Note the 10 allows for the first number, space and a bit of overhead
    // the 13 allows for "255 255 255  ".
    // This is dynamically allocated as we don't know how much space is required
    // until we know the number of colours
    Buffer = SAFE_MALLOC(10 + (NumCols * 13));
    if (Buffer == NULL)
    {
        alert("Error", "Failed to save palette!", NULL, "&Okay", NULL, 'O', 0);
        return;
    }
    sprintf(Buffer, "%d ", NumCols);

    for (j = 0; j < NumCols; j++)
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

    SAFE_FREE(Buffer);
}

// traverses the list of all of the palette to write back to the
// ini file.
void SaveAllPalettes(void)
{
    int i, j;
    int IniFilePalNum = 1;

    // walk through each of the colour planes
    for (i = 0; i < MAX_COL_PLANES; i++)
    {
        // walk through each palette for this plane
        for (j = 0; j < NumColPalettes[i]; j++)
        {
            SavePalette((1 << i), i, j, IniFilePalNum);
            IniFilePalNum++;
        }
    }
}


void MergeFiles(char* ROMName, long size)
{
    char BackupPath[512] = { 0 };
    char PatchPath[512] = { 0 };
    FILE* fpNewRom = NULL;
    FILE* fpBackupRom = NULL;
    FILE* fpPatchRom = NULL;
    char Header[] = "PATCH";
    long Offset = 0;
    long NumBytes = 0;
    long i;
    long BackupPos = 0;
    BYTE ch;

    /* check for rom name in backup directory */
    MakeBackupPath(BackupDir, BackupPath, ROMName, 1);

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

    MakeBackupPath(PatchDir, PatchPath, ROMName, 1);
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
    for (i = 0; i < 5; i++)
    {
        BYTE ch = fgetc(fpPatchRom);
        if (ch != Header[i])
        {
            printf("Invalid Patch file \"%s\"", PatchPath);
            return;
        }
    }

    /* right thats all of the tinkering about done now for the merge */
    for (;;)
    {
        Offset = 0;

        /* get the next 3 bytes for the offset */
        for (i = 0; i < 3; i++)
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
        for (i = BackupPos; i < Offset; i++)
        {
            ch = fgetc(fpBackupRom);
            fputc(ch, fpNewRom);
        }

        /* how many bytes to merge? */
        ch = fgetc(fpPatchRom);
        NumBytes = (256 * ch) + fgetc(fpPatchRom);

        for (i = 0; i < NumBytes; i++)
        {
            ch = fgetc(fpPatchRom);
            fputc(ch, fpNewRom);
        }

        BackupPos = Offset + NumBytes;
    }

    /* now copy any remaining bytes from the backup file */
    fseek(fpBackupRom, BackupPos, SEEK_SET);
    for (i = BackupPos; i < size; i++)
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
    for (i = 0; i < NumGfxRoms; i++)
    {
        MergeFiles(GfxRoms[i].ROMName,
            GfxRoms[i].Size);
    }

#if 0
    /* are there any map roms to create patch files for? */
    if (numDataRoms != 0)
    {
        for (i = 0; i < numDataRoms; i++)
        {
            MergeFiles(Drivers[DataRoms[i].ROMName,
                Drivers[DataRoms[i].Size);
        }
    }
#endif
}
