// palette.c - HARDENED VERSION
//
//  palette functions
//
//  September, 1998
//  jerry@mail.csh.rit.edu
//
//  add &save palette internals by ivan

#include "../INCLUDE/allegro.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "../INCLUDE/general.h"
#include "../INCLUDE/guipal.h"
#include "../INCLUDE/sprtplte.h"
#include "../INCLUDE/util.h"
#include "../INCLUDE/gamedesc.h"
#include "../INCLUDE/editmode.h"
#include "../INCLUDE/palette.h"    // include this after guipal.h!!!
#include "../INCLUDE/inidriv.h"
#include "../INCLUDE/editmode.h"
#include "../INCLUDE/sprite.h"     // for bank numbering

// Memory guard constants

#define MAX_PLANES_SAFE 8  // Safe assumption for maximum color planes

// Safe string copy macro
#define SAFE_STRCOPY(dest, src, dest_size) \
    do { \
        size_t safe_len = strlen(src); \
        if (safe_len >= (dest_size)) { \
            safe_len = (dest_size) - 1; \
            printf("WARNING: Truncating string in %s:%d\n", __FILE__, __LINE__); \
        } \
        memcpy(dest, src, safe_len); \
        dest[safe_len] = '\0'; \
    } while(0)

// Safe bounds checking for palette arrays
#define VALID_PLANE_INDEX(index) ((index) >= 0 && (index) < MAX_PLANES_SAFE)



int pal_fg_color = FIRST_USER_COLOR;
int pal_bg_color = FIRST_USER_COLOR + 1;

PALETTE_SELECT* psel = NULL; // the internal palette (for the gui)
GUARDED_PALETTE* guarded_psel = NULL; // The guarded version

BOOL pal_initted = FALSE;
int current_palette_number = 0;

int edit_color;

extern MYBITMAP* screen;          // Main display bitmap
extern FONT* font;              // Default system font
extern volatile int mouse_b;    // Mouse button state

extern MYBITMAP* create_bitmap(int width, int height);
extern void text_mode(int mode);
extern int alert(const char*, const char*, const char*, const char*, const char*, int, int);
extern int do_dialog(DIALOG* dialog, int focus_obj);
extern int button_dp2_proc(int msg, DIALOG* d, int c);

// Helper function to check palette integrity
BOOL CheckPaletteIntegrity(void) {
    if (guarded_psel == NULL) {
        return FALSE;
    }

    if (guarded_psel->front_guard != PALETTE_FRONT_GUARD) {
        printf("CRITICAL: Palette front guard corrupted! Expected 0x%08lX, got 0x%08lX\n",
            PALETTE_FRONT_GUARD, guarded_psel->front_guard);
        return FALSE;
    }

    if (guarded_psel->rear_guard != PALETTE_REAR_GUARD) {
        printf("CRITICAL: Palette rear guard corrupted! Expected 0x%08lX, got 0x%08lX\n",
            PALETTE_REAR_GUARD, guarded_psel->rear_guard);
        return FALSE;
    }

    return TRUE;
}

// Safe palette pointer accessor
PALETTE_SELECT* GetSafePalette(void) {
    if (!CheckPaletteIntegrity()) {
        printf("ERROR: Palette integrity check failed!\n");
        return NULL;
    }
    return &guarded_psel->data;
}

RGB* GetSafePaletteEntries(void) {
    if (!CheckPaletteIntegrity()) {
        printf("ERROR: Palette integrity check failed in GetSafePaletteEntries!\n");
        return NULL;
    }
    return guarded_psel->palette_entries;
}

void Set_BG_Color(int color)
{
    if (color < 0 || color >= PALETTE_ENTRY_COUNT) {
        printf("WARNING: Set_BG_Color called with invalid color %d\n", color);
        return;
    }
    pal_bg_color = color;
}

void Set_FG_Color(int color)
{
    if (color < 0 || color >= PALETTE_ENTRY_COUNT) {
        printf("WARNING: Set_FG_Color called with invalid color %d\n", color);
        return;
    }
    pal_fg_color = color;
}

int Get_BG_Color(void)
{
    return pal_bg_color;
}

int Get_FG_Color(void)
{
    return pal_fg_color;
}

int psel_callback(DIALOG* d, int color, int mouse)
{
    // Validate color range
    if (color < 0 || color >= PALETTE_ENTRY_COUNT) {
        printf("WARNING: psel_callback called with invalid color %d\n", color);
        return D_O_K;
    }

    int lc;

    // now set the internal 
    if (mouse & MOUSE_FLAG_LEFT_DOWN) {
        lc = Get_BG_Color();
        if (color != lc) {
            Set_BG_Color(color);
        }
    }
    else {
        lc = Get_FG_Color();
        if (color != lc) {
            Set_FG_Color(color);
        }
    }
    return D_O_K;
}

void install_fake_palette(void)
{
    RGB* palette_entries = GetSafePaletteEntries();
    if (palette_entries == NULL) {
        printf("ERROR: Cannot install fake palette - no palette entries\n");
        return;
    }

    PALETTE_SELECT* safe_psel = GetSafePalette();
    if (safe_psel == NULL) {
        printf("ERROR: Cannot install fake palette - no palette structure\n");
        return;
    }

    safe_psel->ncolors = 2;

    palette_entries[FIRST_USER_COLOR].r = 20;
    palette_entries[FIRST_USER_COLOR].g = 20;
    palette_entries[FIRST_USER_COLOR].b = 30;

    palette_entries[FIRST_USER_COLOR + 1].r = 50;
    palette_entries[FIRST_USER_COLOR + 1].g = 30;
    palette_entries[FIRST_USER_COLOR + 1].b = 30;

    Set_BG_Color(FIRST_USER_COLOR);
    Set_FG_Color(FIRST_USER_COLOR + 1);
}

void debug_palette_data(int palette_index, int pal_no) {
    printf("\n=== PALETTE DATA DEBUG ===\n");
    printf("Checking ColPalettes[%d] for palette %d\n", palette_index, pal_no);

    if (palette_index < 0 || palette_index >= MAX_COL_PLANES) {
        printf("ERROR: Invalid palette index %d\n", palette_index);
        return;
    }

    if (ColPalettes[palette_index] == NULL) {
        printf("ERROR: ColPalettes[%d] is NULL\n", palette_index);
        return;
    }

    int num_colors = 1 << palette_index;
    int max_palettes = NumColPalettes[palette_index];

    if (pal_no < 0 || pal_no >= max_palettes) {
        printf("ERROR: Invalid palette number %d (max=%d)\n", pal_no, max_palettes);
        return;
    }

    printf("Number of colors: %d\n", num_colors);
    printf("Number of palettes at this index: %d\n", max_palettes);

    for (int i = 0; i < num_colors; i++) {
        int src_index = pal_no * num_colors + i;
        if (src_index < max_palettes * num_colors) {
            RGB color = ColPalettes[palette_index][src_index];
            printf("  Color %d: R=%d, G=%d, B=%d\n", i, color.r, color.g, color.b);
        }
    }

    printf("=== END PALETTE DATA DEBUG ===\n\n");
}

void install_palette(int pal_no) // installs the selected palette number
{
    printf("DEBUG: install_palette called with pal_no=%d\n", pal_no);

    // a few pointer checks
    if (GameDriverLoaded == FALSE)
    {
        printf("DEBUG: Game not loaded, installing fake palette\n");
        install_fake_palette();
        return;
    }

    if (GfxBankExtraInfo == NULL)
    {
        printf("DEBUG: GfxBankExtraInfo is NULL, installing fake palette\n");
        install_fake_palette();
        return;
    }

    if (!CheckPaletteIntegrity())
    {
        printf("DEBUG: Palette integrity check failed, installing fake palette\n");
        install_fake_palette();
        return;
    }

    PALETTE_SELECT* safe_psel = GetSafePalette();
    RGB* palette_entries = GetSafePaletteEntries();
    if (safe_psel == NULL || palette_entries == NULL) {
        printf("ERROR: Cannot install palette - safe pointers are NULL\n");
        install_fake_palette();
        return;
    }

    // Validate currentGfxBank
    if (currentGfxBank < 0 || currentGfxBank >= NumGfxBanks) {
        printf("ERROR: Invalid currentGfxBank %d (NumGfxBanks=%d)\n", currentGfxBank, NumGfxBanks);
        install_fake_palette();
        return;
    }

    int planes = GfxBankExtraInfo[currentGfxBank].planes;
    int num_colors = 1 << planes;  // 2^planes = number of colors

    // Calculate the correct palette index based on number of colors
    int palette_index = planes;
    while ((1 << palette_index) < num_colors && palette_index < MAX_COL_PLANES - 1) {
        palette_index++;
    }

    printf("DEBUG: Planes=%d, Colors=%d, PaletteIndex=%d\n", planes, num_colors, palette_index);

    // Validate palette index
    if (palette_index < 0 || palette_index >= MAX_COL_PLANES) {
        printf("ERROR: Invalid palette index %d\n", palette_index);
        install_fake_palette();
        return;
    }

    // Validate palette number
    if (pal_no < 0 || pal_no >= NumColPalettes[palette_index]) {
        printf("WARNING: Invalid palette number %d (max=%d), using 0\n",
            pal_no, NumColPalettes[palette_index]);
        pal_no = 0;
    }

    current_palette_number = pal_no;
    safe_psel->ncolors = num_colors;

    // Validate num_colors
    if (num_colors <= 0 || num_colors > PALETTE_ENTRY_COUNT) {
        printf("ERROR: Invalid num_colors %d\n", num_colors);
        install_fake_palette();
        return;
    }

    // Validate ColPalettes
    if (ColPalettes == NULL || ColPalettes[palette_index] == NULL) {
        printf("ERROR: ColPalettes[%d] is NULL\n", palette_index);
        install_fake_palette();
        return;
    }

    printf("DEBUG: Installing palette %d for %d colors\n", pal_no, num_colors);

    debug_palette_data(palette_index, pal_no);

    // Copy palette colors - FIXED indexing
    for (int i = 0; i < num_colors; i++)
    {
        int dest_index = FIRST_USER_COLOR + i;

        // Validate destination index
        if (dest_index < 0 || dest_index >= PALETTE_ENTRY_COUNT) {
            printf("WARNING: Destination palette index %d out of bounds\n", dest_index);
            continue;
        }

        // Calculate source index - palette data is stored as [palette_index][palette_number * num_colors + color_index]
        int src_index = pal_no * num_colors + i;

        // Validate source index
        if (src_index < 0 || src_index >= (NumColPalettes[palette_index] * num_colors)) {
            printf("WARNING: Source palette index %d out of bounds (max=%d)\n",
                src_index, NumColPalettes[palette_index] * num_colors);
            continue;
        }

        // Copy the color
        palette_entries[dest_index].r = ColPalettes[palette_index][src_index].r;
        palette_entries[dest_index].g = ColPalettes[palette_index][src_index].g;
        palette_entries[dest_index].b = ColPalettes[palette_index][src_index].b;

        printf("DEBUG: Color %d: RGB(%d, %d, %d) from ColPalettes[%d][%d]\n",
            i, palette_entries[dest_index].r, palette_entries[dest_index].g,
            palette_entries[dest_index].b, palette_index, src_index);
    }

    // Apply the palette to the system
    printf("DEBUG: Setting palette range from %d to %d\n",
        FIRST_USER_COLOR, FIRST_USER_COLOR + num_colors - 1);

    set_palette_range(palette_entries, FIRST_USER_COLOR, FIRST_USER_COLOR + num_colors - 1, 1);

    printf("DEBUG: install_palette completed successfully\n");
}

// In palette.c, add a critical section to prevent reentrancy
BOOL palette_operation_in_progress = FALSE;

void DeInit_Palette(void)
{
    // Prevent reentrancy
    if (palette_operation_in_progress) {
        printf("WARNING: DeInit_Palette called recursively, skipping\n");
        return;
    }

    palette_operation_in_progress = TRUE;
    printf("DEBUG: DeInit_Palette called, pal_initted=%d, guarded_psel=%p\n", pal_initted, guarded_psel);

    if (!pal_initted && guarded_psel == NULL) {
        printf("DEBUG: Palette already deinitialized\n");
        palette_operation_in_progress = FALSE;
        return;
    }

    // Free the guarded palette structure if it exists
    if (guarded_psel != NULL) {
        // Check integrity before freeing
        if (CheckPaletteIntegrity()) {
            printf("DEBUG: Palette integrity OK before deinit\n");
        }
        else {
            printf("WARNING: Palette integrity compromised before deinit\n");
        }

        printf("DEBUG: Freeing guarded palette structure at %p\n", guarded_psel);

        // Check if the pointer looks valid before freeing
        uintptr_t ptr_val = (uintptr_t)guarded_psel;
        if (ptr_val >= 0x1000 && ptr_val <= 0x7FFFFFFF) {
            SAFE_FREE(guarded_psel);
        }
        else {
            printf("WARNING: Invalid guarded_psel pointer %p, skipping free\n", guarded_psel);
        }
        guarded_psel = NULL;
        psel = NULL; // Also clear the external pointer
    }
    else {
        printf("DEBUG: No guarded palette structure to free\n");
    }

    pal_initted = FALSE;
    palette_operation_in_progress = FALSE;
    printf("DEBUG: DeInit_Palette completed\n");
}

void Init_Palette(void)
{
    // Prevent reentrancy
    if (palette_operation_in_progress) {
        printf("WARNING: Init_Palette called recursively, skipping\n");
        return;
    }

    palette_operation_in_progress = TRUE;
    printf("DEBUG: Init_Palette called, pal_initted=%d, guarded_psel=%p\n", pal_initted, guarded_psel);

    // Clean up any existing palette first
    if (pal_initted || guarded_psel != NULL) {
        printf("DEBUG: Cleaning up existing palette before reinitialization\n");
        DeInit_Palette();
    }

    // Allocate the guarded palette structure
    guarded_psel = (GUARDED_PALETTE*)SAFE_MALLOC(sizeof(GUARDED_PALETTE));
    if (guarded_psel == NULL) {
        printf("ERROR: Failed to allocate GUARDED_PALETTE structure!\n");
        pal_initted = FALSE;
        palette_operation_in_progress = FALSE;
        return;
    }
    printf("DEBUG: Allocated guarded palette at %p\n", guarded_psel);

    // Initialize memory guards
    guarded_psel->front_guard = PALETTE_FRONT_GUARD;
    guarded_psel->rear_guard = PALETTE_REAR_GUARD;

    // Initialize the palette structure
    guarded_psel->data.basecolor = FIRST_USER_COLOR;
    guarded_psel->data.ncolors = PALETTE_ENTRY_COUNT;
    guarded_psel->data.palette = guarded_psel->palette_entries; // Point to our internal array

    // Initialize palette entries to safe values
    memset(guarded_psel->palette_entries, 0, sizeof(RGB) * PALETTE_ENTRY_COUNT);

    // Set the external pointer
    psel = &guarded_psel->data;

    // Verify integrity
    if (!CheckPaletteIntegrity()) {
        printf("ERROR: Palette integrity check failed after initialization!\n");
        DeInit_Palette();
        palette_operation_in_progress = FALSE;
        return;
    }

    // Set up the palette system
    install_palette(0);
    Set_BG_Color(FIRST_USER_COLOR);
    Set_FG_Color(FIRST_USER_COLOR + 1);

    pal_initted = TRUE;
    palette_operation_in_progress = FALSE;
    printf("DEBUG: Init_Palette completed successfully\n");
}

int pal_plus(DIALOG* d)
{
    if (GameDriverLoaded == FALSE) {
        printf("DEBUG: pal_plus - game not loaded\n");
        return D_O_K;
    }

    if (!CheckPaletteIntegrity()) {
        printf("ERROR: pal_plus - palette integrity check failed\n");
        return D_O_K;
    }

    int planes = GfxBankExtraInfo[currentGfxBank].planes;
    if (!VALID_PLANE_INDEX(planes)) {
        printf("ERROR: pal_plus - invalid planes index %d\n", planes);
        return D_O_K;
    }

    int max_palettes = NumColPalettes[planes];
    if (current_palette_number < max_palettes - 1) {
        current_palette_number++;
        install_palette(current_palette_number);
        return D_REDRAW;
    }
    else {
        printf("DEBUG: pal_plus - already at maximum palette %d\n", current_palette_number);
        return D_O_K;
    }
}

int palette_inc(void)
{
    return pal_plus(NULL);
}

int pal_minus(DIALOG* d)
{
    if (GameDriverLoaded == FALSE) {
        printf("DEBUG: pal_minus - game not loaded\n");
        return D_O_K;
    }

    if (!CheckPaletteIntegrity()) {
        printf("ERROR: pal_minus - palette integrity check failed\n");
        return D_O_K;
    }

    if (current_palette_number > 0) {
        current_palette_number--;
        install_palette(current_palette_number);
        return D_REDRAW;
    }
    else {
        printf("DEBUG: pal_minus - already at minimum palette 0\n");
        return D_O_K;
    }
}

int palette_dec(void)
{
    return pal_minus(NULL);
}

void palette_reset(void)
{
    if (GameDriverLoaded == FALSE) {
        printf("DEBUG: palette_reset - game not loaded\n");
        return;
    }

    if (!CheckPaletteIntegrity()) {
        printf("ERROR: palette_reset - palette integrity check failed\n");
        return;
    }

    install_palette(0);
}

////////////////////////////////////////////////////////////////////////////////
// add a new colour palette

// Create the new memory for the enhanced sized palette if fails then the old
// palette is still valid
// failure is denoted by -1 being returned. Other values reflect a the new
// palette number
int CreateNewPaletteWithCurrentColours(void)
{
    printf("DEBUG: CreateNewPaletteWithCurrentColours called\n");

    // do a check before we do anything here
    if (GameDriverLoaded == FALSE) {
        printf("ERROR: Game not loaded\n");
        return -1;
    }

    if (!CheckPaletteIntegrity()) {
        printf("ERROR: Palette integrity check failed\n");
        return -1;
    }

    int NumCols = 1 << GfxBankExtraInfo[currentGfxBank].planes;
    int PalSize = NumCols * sizeof(RGB);
    int PalIndex = GfxBankExtraInfo[currentGfxBank].planes;

    // Validate indices
    if (!VALID_PLANE_INDEX(PalIndex)) {
        printf("ERROR: Invalid PalIndex %d\n", PalIndex);
        return -1;
    }

    if (NumColPalettes[PalIndex] < 0) {
        printf("ERROR: Invalid NumColPalettes[%d] = %d\n", PalIndex, NumColPalettes[PalIndex]);
        return -1;
    }

    // create memory for larger palette
    RGB* tPal = (RGB*)SAFE_MALLOC(PalSize * (NumColPalettes[PalIndex] + 1));
    if (tPal == NULL) {
        printf("ERROR: Failed to allocate new palette memory\n");
        return -1;
    }
    printf("DEBUG: Allocated new palette at %p\n", tPal);

    // copy old palette to new memory
    if (ColPalettes[PalIndex] != NULL) {
        memcpy(tPal, ColPalettes[PalIndex], PalSize * NumColPalettes[PalIndex]);
    }
    else {
        printf("WARNING: ColPalettes[%d] was NULL, initializing new palette\n", PalIndex);
        memset(tPal, 0, PalSize * (NumColPalettes[PalIndex] + 1));
    }

    NumColPalettes[PalIndex]++;

    // free old memory and copy pointer to new palette
    if (ColPalettes[PalIndex] != NULL) {
        SAFE_FREE(ColPalettes[PalIndex]);
    }
    ColPalettes[PalIndex] = tPal;

    // copy current palette over the new one
    for (int j = 0; j < NumCols; j++) {
        int NewOffset = NumCols * (NumColPalettes[PalIndex] - 1);
        int CurrentOffset = NumCols * current_palette_number;

        // Validate indices
        if (NewOffset + j < 0 || NewOffset + j >= NumCols * NumColPalettes[PalIndex]) {
            printf("WARNING: New palette index %d out of bounds\n", NewOffset + j);
            continue;
        }

        if (CurrentOffset + j < 0 || CurrentOffset + j >= NumCols * (NumColPalettes[PalIndex] - 1)) {
            printf("WARNING: Current palette index %d out of bounds\n", CurrentOffset + j);
            continue;
        }

        ColPalettes[PalIndex][j + NewOffset].r = ColPalettes[PalIndex][j + CurrentOffset].r;
        ColPalettes[PalIndex][j + NewOffset].g = ColPalettes[PalIndex][j + CurrentOffset].g;
        ColPalettes[PalIndex][j + NewOffset].b = ColPalettes[PalIndex][j + CurrentOffset].b;
    }

    // save the new palette to the ini file
    SaveAllPalettes();

    // return the new palette number
    return NumColPalettes[PalIndex] - 1;
}

// ... rest of the functions remain the same as in the previous version ...


int palette_add_new(void)
{
    printf("DEBUG: palette_add_new called\n");

    if (GameDriverLoaded == FALSE) {
        printf("ERROR: Game not loaded\n");
        return D_O_K;
    }

    // reallocate the memory to make space for the new palette
    int NewPaletteNum = CreateNewPaletteWithCurrentColours();
    if (NewPaletteNum == -1) {
        // the current palette is still intact therefore just alert the user
        // and carry on
        alert("", "Can't create new palette", NULL, "&Okay", NULL, 'O', 0);
        return D_O_K;
    }

    // swtich to the new palette
    current_palette_number = NewPaletteNum;
    install_palette(current_palette_number);

    return D_REDRAW;
}

//------------------------------------------------------------------------------
// palette editor functions:

extern DIALOG palette_editor[];
void adjust_sliders(void);

void refresh_palette_settings(void)
{
    if (!CheckPaletteIntegrity()) {
        printf("ERROR: refresh_palette_settings - palette integrity check failed\n");
        return;
    }

    PALETTE_SELECT* safe_psel = GetSafePalette();
    if (safe_psel == NULL) {
        printf("ERROR: refresh_palette_settings - safe_psel is NULL\n");
        return;
    }

    // install the palette...
    set_palette_range(safe_psel->palette, safe_psel->basecolor,
        safe_psel->basecolor + safe_psel->ncolors, 1);

    // redraw text on the screen
    show_mouse(NULL, "refresh_palette_settings 1");
    if (palette_editor[9].proc != NULL) {
        palette_editor[9].proc(MSG_DRAW, &palette_editor[9], 0);
    }
    show_mouse(screen, "refresh_palette_settings 2");
}

int pal_edit_image_grab(DIALOG* d)
{
    char pal_path[255];
    char* pos = NULL;
    MYBITMAP* the_bitmap;
    RGB pall[256];
    int NumCols = 1 << GfxBankExtraInfo[currentGfxBank].planes;
    int i;

    if (!CheckPaletteIntegrity()) {
        printf("ERROR: pal_edit_image_grab - palette integrity check failed\n");
        return D_O_K;
    }

    RGB* palette_entries = GetSafePaletteEntries();
    if (palette_entries == NULL) {
        printf("ERROR: pal_edit_image_grab - palette_entries is NULL\n");
        return D_O_K;
    }

    // Initialize fallback palette
    for (i = 0; i < 255; i++) {
        pall[i].r = pall[i].g = pall[i].b = (i & 1) ? 0x42 : 0;
    }

    // Safe string copy
    const char* config_path = get_config_string("System", "Pal_Grab_Path", ".");
    SAFE_STRCOPY(pal_path, config_path, sizeof(pal_path));
    put_backslash(pal_path, sizeof(pal_path));

    if (file_select("Choose an image...", pal_path, "BMP;LBM;PCX;TGA")) {
        busy();
        the_bitmap = load_bitmap(pal_path, pall);
        if (the_bitmap) {
            destroy_bitmap(the_bitmap); // eh, we just want the palette.

            // Copy the loaded palette, with bounds checking
            for (i = 0; i < NumCols && i < 256; i++) {
                palette_entries[FIRST_USER_COLOR + i].r = pall[i].r;
                palette_entries[FIRST_USER_COLOR + i].g = pall[i].g;
                palette_entries[FIRST_USER_COLOR + i].b = pall[i].b;
            }
            refresh_palette_settings();
            adjust_sliders();
        }
        else {
            not_busy();
            alert("Unable to load image file", pal_path, NULL,
                "&Bummer", NULL, 'O', 0);
        }

        pos = get_filename(pal_path);
        if (pos != NULL) {
            *pos = '\0';
            set_config_string("System", "Pal_Grab_Path", pal_path);
        }
    }
    not_busy();

    return D_REDRAW;
}

int pal_edit_cancel(DIALOG* d)
{
    // restore from before changes...
    int NumCols = 1 << GfxBankExtraInfo[currentGfxBank].planes;
    int PalIndex = GfxBankExtraInfo[currentGfxBank].planes;
    int i;

    if (!CheckPaletteIntegrity()) {
        printf("ERROR: pal_edit_cancel - palette integrity check failed\n");
        return D_EXIT;
    }

    RGB* palette_entries = GetSafePaletteEntries();
    if (palette_entries == NULL) {
        printf("ERROR: pal_edit_cancel - palette_entries is NULL\n");
        return D_EXIT;
    }

    // Validate ColPalettes
    if (ColPalettes[PalIndex] == NULL) {
        printf("ERROR: pal_edit_cancel - ColPalettes[%d] is NULL\n", PalIndex);
        return D_EXIT;
    }

    for (i = 0; i < NumCols; i++) {
        int src_index = NumCols * current_palette_number + i;
        int dest_index = FIRST_USER_COLOR + i;

        // Validate indices
        if (src_index >= 0 && src_index < NumCols * NumColPalettes[PalIndex] &&
            dest_index >= 0 && dest_index < PALETTE_ENTRY_COUNT) {
            palette_entries[dest_index].r = ColPalettes[PalIndex][src_index].r;
            palette_entries[dest_index].g = ColPalettes[PalIndex][src_index].g;
            palette_entries[dest_index].b = ColPalettes[PalIndex][src_index].b;
        }
    }
    return D_EXIT;
}

int pal_edit_ok(DIALOG* d)
{
    int NumCols = 1 << GfxBankExtraInfo[currentGfxBank].planes;
    int PalIndex = GfxBankExtraInfo[currentGfxBank].planes;
    int i;

    if (!CheckPaletteIntegrity()) {
        printf("ERROR: pal_edit_ok - palette integrity check failed\n");
        return D_EXIT;
    }

    RGB* palette_entries = GetSafePaletteEntries();
    if (palette_entries == NULL) {
        printf("ERROR: pal_edit_ok - palette_entries is NULL\n");
        return D_EXIT;
    }

    // Validate ColPalettes
    if (ColPalettes[PalIndex] == NULL) {
        printf("ERROR: pal_edit_ok - ColPalettes[%d] is NULL\n", PalIndex);
        return D_EXIT;
    }

    for (i = 0; i < NumCols; i++) {
        int dest_index = NumCols * current_palette_number + i;
        int src_index = FIRST_USER_COLOR + i;

        // Validate indices
        if (dest_index >= 0 && dest_index < NumCols * NumColPalettes[PalIndex] &&
            src_index >= 0 && src_index < PALETTE_ENTRY_COUNT) {
            ColPalettes[PalIndex][dest_index].r = palette_entries[src_index].r;
            ColPalettes[PalIndex][dest_index].g = palette_entries[src_index].g;
            ColPalettes[PalIndex][dest_index].b = palette_entries[src_index].b;
        }
    }

    // save palettes to the ini file
    SaveAllPalettes();

    return D_EXIT;
}

void adjust_sliders(void)
{
    if (!CheckPaletteIntegrity()) {
        printf("ERROR: adjust_sliders - palette integrity check failed\n");
        return;
    }

    RGB* palette_entries = GetSafePaletteEntries();
    if (palette_entries == NULL) {
        printf("ERROR: adjust_sliders - palette_entries is NULL\n");
        return;
    }

    // Validate edit_color range
    if (edit_color < 0 || edit_color >= PALETTE_ENTRY_COUNT) {
        printf("WARNING: adjust_sliders - invalid edit_color %d\n", edit_color);
        return;
    }

    palette_editor[3].d2 = palette_entries[edit_color].r;
    palette_editor[4].d2 = palette_entries[edit_color].g;
    palette_editor[5].d2 = palette_entries[edit_color].b;
}

int psel_edit_callback(DIALOG* d, int color, int mouse)
{
    // Validate color range
    if (color < 0 || color >= PALETTE_ENTRY_COUNT) {
        printf("WARNING: psel_edit_callback - invalid color %d\n", color);
        return D_O_K;
    }

    edit_color = color;

    // now adjust the sliders...
    adjust_sliders();

    while (mouse_b);

    return D_REDRAW;
}

int r_slider(void* dp3, int d2)
{
    if (!CheckPaletteIntegrity()) {
        printf("ERROR: r_slider - palette integrity check failed\n");
        return D_O_K;
    }

    RGB* palette_entries = GetSafePaletteEntries();
    if (palette_entries == NULL) {
        printf("ERROR: r_slider - palette_entries is NULL\n");
        return D_O_K;
    }

    // Validate edit_color range
    if (edit_color < 0 || edit_color >= PALETTE_ENTRY_COUNT) {
        printf("WARNING: r_slider - invalid edit_color %d\n", edit_color);
        return D_O_K;
    }

    palette_entries[edit_color].r = d2;
    refresh_palette_settings();
    return D_O_K;
}

int g_slider(void* dp3, int d2)
{
    if (!CheckPaletteIntegrity()) {
        printf("ERROR: g_slider - palette integrity check failed\n");
        return D_O_K;
    }

    RGB* palette_entries = GetSafePaletteEntries();
    if (palette_entries == NULL) {
        printf("ERROR: g_slider - palette_entries is NULL\n");
        return D_O_K;
    }

    // Validate edit_color range
    if (edit_color < 0 || edit_color >= PALETTE_ENTRY_COUNT) {
        printf("WARNING: g_slider - invalid edit_color %d\n", edit_color);
        return D_O_K;
    }

    palette_entries[edit_color].g = d2;
    refresh_palette_settings();
    return D_O_K;
}

int b_slider(void* dp3, int d2)
{
    if (!CheckPaletteIntegrity()) {
        printf("ERROR: b_slider - palette integrity check failed\n");
        return D_O_K;
    }

    RGB* palette_entries = GetSafePaletteEntries();
    if (palette_entries == NULL) {
        printf("ERROR: b_slider - palette_entries is NULL\n");
        return D_O_K;
    }

    // Validate edit_color range
    if (edit_color < 0 || edit_color >= PALETTE_ENTRY_COUNT) {
        printf("WARNING: b_slider - invalid edit_color %d\n", edit_color);
        return D_O_K;
    }

    palette_entries[edit_color].b = d2;
    refresh_palette_settings();
    return D_O_K;
}

int pal_screen_text(int msg, DIALOG* d, int c)
{
    if (msg == MSG_DRAW)
    {
        // draw up all of the text onto the screen...
        text_mode(GUI_BACK);

        textprintf_centre(screen, font, d->x + 58, d->y + 97, GUI_FORE,
            " Color %d ", edit_color - psel->basecolor);

        textprintf_centre(screen, font, d->x + 118, d->y + 97, GUI_FORE,
            " %d ", palette_editor[3].d2);
        textprintf_centre(screen, font, d->x + 143, d->y + 97, GUI_FORE,
            " %d ", palette_editor[4].d2);
        textprintf_centre(screen, font, d->x + 168, d->y + 97, GUI_FORE,
            " %d ", palette_editor[5].d2);

        border_3d(screen, d->x + 23, d->y + 94, 68, 14, 0);
        border_3d(screen, d->x + 106, d->y + 94, 22, 14, 0);
        border_3d(screen, d->x + 131, d->y + 94, 22, 14, 0);
        border_3d(screen, d->x + 156, d->y + 94, 22, 14, 0);
    }

    return D_O_K;
}

int palette_edit(void)
{
    if (GameDriverLoaded == FALSE) {
        printf("DEBUG: palette_edit - game not loaded\n");
        return D_O_K;
    }

    if (!CheckPaletteIntegrity()) {
        printf("ERROR: palette_edit - palette integrity check failed\n");
        return D_O_K;
    }

    PALETTE_SELECT* safe_psel = GetSafePalette();
    if (safe_psel == NULL) {
        printf("ERROR: palette_edit - safe_psel is NULL\n");
        return D_O_K;
    }

    // create a dialog with a color selector, and 3 sliders for red/green/blue
    edit_color = safe_psel->basecolor;
    adjust_sliders();

    position_dialog(palette_editor, SCREEN_W - palette_editor[0].w - 2, 2);
    set_dialog_color(palette_editor, GUI_FORE, GUI_BACK);
    palette_editor[3].fg = COLOR_RED;
    palette_editor[4].fg = COLOR_GREEN;
    palette_editor[5].fg = COLOR_BLUE;
    do_dialog(palette_editor, -1);

    return D_REDRAW;
}

static DIALOG palette_editor[] =
{
    /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)
               (key) (flags)  (d1)  (d2)  (dp) */
    { d_shadow_box_proc, 0,    0,    200,  170,  0,    0,
                 0,    0,       0,    0,    NULL },
    { d_ctext_proc,      100,  8,    1,    1,    0,    0,
                 0,    0,       0,    0,   "Palette Editor" },

    { pal_select_proc,  25, 25, (4 * 15) + 4, (4 * 15) + 4, GUI_FORE, GUI_BACK,
                0, 0, 0, 0, &psel, psel_edit_callback },

    { d_slider_proc,   110, 25, 15, 64, GUI_FORE, GUI_BACK,
                 0, 0, 63, 40, NULL, r_slider},
    { d_slider_proc,   135, 25, 15, 64, GUI_FORE, GUI_BACK,
                 0, 0, 63, 40, NULL, g_slider},
    { d_slider_proc,   160, 25, 15, 64, GUI_FORE, GUI_BACK,
                 0, 0, 63, 40, NULL, b_slider},

    { button_dp2_proc,   48,  120,  100,   16,   0,    0,
                 'I',    D_EXIT,  0,    0,  "&Image Grab", pal_edit_image_grab },

    { button_dp2_proc,   15,  140,  80,   16,   0,    0,
                 'A',    D_EXIT,  0,    0,    "&Accept", pal_edit_ok },

    { button_dp2_proc,   103,  140,  80,   16,   0,    0,
                   27,   D_EXIT,  0,    0,    "Cancel", pal_edit_cancel },

    { pal_screen_text },

    { NULL }
};