// fileio.c
//
//  The file io functions
//
//  October, 1998
//  jerry@mail.csh.rit.edu

#include "../INCLUDE/allegro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../INCLUDE/general.h"
#include "../INCLUDE/drivsel.h"
#include "../INCLUDE/INIDriv.h"
#include "../INCLUDE/sprtplte.h"
#include "../INCLUDE/guipal.h"
#include "../INCLUDE/gamedesc.h"
#include "../INCLUDE/palette.h"
#include "../INCLUDE/editmode.h"
#include "../INCLUDE/editmenu.h"
#include "../INCLUDE/sprite.h"
#include "../INCLUDE/coding.h"
#include <Windows.h>

extern MYBITMAP* current_sprite;
extern MYBITMAP* create_bitmap(int width, int height);
extern int alert(const char*, const char*, const char*, const char*, const char*, int, int);

void unload_driver(void)
{
	printf("DEBUG: unload_driver called\n");

	// Only unload if a driver is actually loaded
	if (GameDriverLoaded) {
		printf("DEBUG: Freeing driver data\n");
		FreeDriver();
		GameDriverLoaded = FALSE;  // Important: mark as unloaded

		// After FreeDriver(), sprite_bank is likely invalid
		// Don't free it again - just set to NULL
		sprite_bank = NULL;
	}
	else {
		printf("DEBUG: No driver loaded, skipping FreeDriver\n");
	}

	InitialiseGameDesc();

	// sprite_bank should already be NULL if we freed the driver
	// Only free it if we're in some unexpected state
	if (sprite_bank) {
		printf("WARNING: sprite_bank still exists after driver unload, freeing it\n");
		SAFE_FREE(sprite_bank);
		sprite_bank = NULL;
	}

	// Only destroy and recreate current_sprite if it exists
	if (current_sprite) {
		destroy_bitmap(current_sprite);
		current_sprite = NULL;
	}

	current_sprite = create_bitmap(ED_DEF_SIZE, ED_DEF_SIZE);
	if (current_sprite) {
		clear_to_color(current_sprite, FIRST_USER_COLOR);
		printf("DEBUG: Recreated current_sprite\n");
	}
	else {
		printf("ERROR: Failed to recreate current_sprite!\n");
	}

	printf("DEBUG: unload_driver completed\n");
}

// Global variable to track menu integrity
static unsigned long menu_canary = 0xDEADBEEF;

void check_menu_integrity(void) {
	if (menu_canary != 0xDEADBEEF) {
		printf("CRITICAL: Menu memory corrupted! Canary: 0x%08lX\n", menu_canary);
		// Try to restore it
		menu_canary = 0xDEADBEEF;
	}
}

// Call this before any menu operations
#define CHECK_MENU_INTEGRITY() check_menu_integrity()

BOOL palette_needs_reinit = FALSE;

// In fileio.c, modify try_loading_the_driver
void try_loading_the_driver(char* drivername)
{
	printf("DEBUG: try_loading_the_driver called with: %s\n", drivername);

	// First unload any current driver
	unload_driver();

	if (LoadDriver(drivername) == TRUE)
	{
		printf("DEBUG: Driver loaded successfully\n");
		// setup the sprite palette & stuff..
		set_Gfx_bank(0);

		// Defer palette initialization to avoid menu context issues
		printf("DEBUG: Driver loaded, graphics bank set - palette will be initialized on next redraw\n");

		// Instead of calling Init_Palette() directly here, set a flag
		// and let the main dialog handle it during the next redraw
		palette_needs_reinit = TRUE;
	}
	else {
		printf("DEBUG: Driver loading failed\n");
		unload_driver();
	}
}
//
/// FILE MENU FUNCTIONS
//
// Add this to track sprite_bank ownership
void debug_sprite_bank_state(void) {
	printf("=== SPRITE_BANK DEBUG ===\n");
	printf("sprite_bank pointer: %p\n", sprite_bank);
	printf("GameDriverLoaded: %d\n", GameDriverLoaded);
	printf("currentGfxBank: %d\n", currentGfxBank);

	if (sprite_bank) {
		printf("sprite_bank details:\n");
		printf("  n_total: %d\n", sprite_bank->n_total);
		printf("  sprite_w: %d, sprite_h: %d\n", sprite_bank->sprite_w, sprite_bank->sprite_h);
		printf("  bmp: %p\n", sprite_bank->bmp);
	}
	printf("=========================\n");
}

// Call this before and after driver operations
// 
extern int game_change_requested;
extern char pending_driver_load[256];

// Instrumented file_game with memory checks
int file_game(void) {
	printf("=== FILE_GAME START (with memory checks) ===\n");
	check_all_allocations("file_game start");

	if (GameDriverLoaded) {
		if (alert("ARE YOU SURE?",
			"You can't undo this!",
			"(even if we had an undo)",
			"&Continue", "Cancel", 0, 0) == 2) {
			printf("User cancelled\n");
			return D_O_K;
		}
	}

	printf("=== Unloading driver ===\n");
	check_all_allocations("before unload_driver");

	unload_driver();

	check_all_allocations("after unload_driver");

	printf("=== Showing game driver selector ===\n");
	check_all_allocations("before game_driver_selector");

	game_driver_selector();

	check_all_allocations("after game_driver_selector");

	busy();

	if (SDL_strlen(selected_filename)) {
		printf("=== Loading driver: %s ===\n", selected_filename);
		check_all_allocations("before try_loading_the_driver");

		try_loading_the_driver(selected_filename);

		check_all_allocations("after try_loading_the_driver");

		if (!GameDriverLoaded) {
			printf("=== Driver failed to load ===\n");
			not_busy();
			alert("Error loading driver:", selected_filename, "Sorry.",
				"&Okay", NULL, 'O', 0);
			unload_driver();
		}
	}

	printf("=== Calling not_busy ===\n");
	check_all_allocations("before not_busy in file_game");

	not_busy();

	check_all_allocations("after not_busy in file_game");

	printf("=== FILE_GAME END ===\n");
	return D_REDRAW;
}


int file_revert(void)
{
    // re-load the data from the roms

    if(GameDriverLoaded)
    {
	if (alert("ARE YOU SURE?", 
	          "You can't undo this!", 
	          "(even if we had an undo)", 
	      "&Continue", "Cancel", 0, 0) == 1)
	{
	    busy();
	    unload_driver();

	    if (LoadDriver(INI_Driver_Path) == TRUE)
	    {
		// setup the sprite palette & stuff..
		set_Gfx_bank(0);

		// load in the palette
		Init_Palette();
	    }

	    not_busy();
	    return D_REDRAW;
	}

    } else {
	alert("Cannot revert graphics!", "No romdata loaded!", "Sorry.", 
	      "&Okay", NULL, 'O', 0);
    }

    return D_O_K;
}

int file_exit(void)
{
    if (GameDriverLoaded)
    {
	if (alert("Are you sure you want", 
	          "to exit and abandon any", 
	          "changes you've made?", 
		  "&Cancel", "E&xit", 'c', 'x') == 1)
	    return(D_O_K);
	else
	{
#ifndef NO_FADE
	    fade_out(FADE_SPEED);
#endif
	    return(D_EXIT);
	}
    } 
#ifndef NO_FADE
    fade_out(FADE_SPEED);
#endif
    return(D_EXIT); // note: if this is D_O_K, then you need to load a driver
                    //       in order to quit.  yow!
}


// Function to set the current sprite being edited
void SetCurrentSpriteIndex(int index) {
	if (index >= 0 && GameDriverLoaded && currentGfxBank >= 0) {
		SPRITE_PALETTE* sp = &GfxBanks[currentGfxBank];
		if (index < sp->n_total) {
			current_sprite_index = index;
			printf("Set current sprite index to %d\n", index);

			// Load the sprite from master bitmap to current_sprite for editing
			if (sp->bmp && current_sprite) {
				int master_x = index * sp->sprite_w;
				blit(sp->bmp, current_sprite, master_x, 0, 0, 0,
					sp->sprite_w, sp->sprite_h);
				printf("Loaded sprite %d from master bitmap\n", index);
			}
		}
	}
}

void CommitCurrentSpriteToMaster(void) {
	if (!GameDriverLoaded || currentGfxBank < 0 || !current_sprite) {
		printf("Cannot commit: Invalid state\n");
		return;
	}

	SPRITE_PALETTE* sp = &GfxBanks[currentGfxBank];
	int master_x = current_sprite_index * sp->sprite_w;  // Use current_sprite_index

	printf("Committing sprite %d to master bitmap at x=%d\n",
		current_sprite_index, master_x);

	if (sp->bmp && current_sprite) {
		blit(current_sprite, sp->bmp, 0, 0, master_x, 0,
			sp->sprite_w, sp->sprite_h);
		printf("Commit successful for sprite %d\n", current_sprite_index);
	}
	else {
		printf("Commit failed: sp->bmp=%p, current_sprite=%p\n",
			sp->bmp, current_sprite);
	}
}

void VerifyCurrentSpriteCommit(void) {
	if (!GameDriverLoaded || currentGfxBank < 0) {
		return;
	}

	SPRITE_PALETTE* sp = &GfxBanks[currentGfxBank];
	int master_x = current_sprite_index * sp->sprite_w;

	printf("=== VERIFYING COMMIT FOR SPRITE %d ===\n", current_sprite_index);

	if (sp->bmp && current_sprite) {
		// Compare a few pixels between current_sprite and master bitmap
		printf("Comparing current_sprite with master bitmap at x=%d:\n", master_x);

		int mismatches = 0;
		for (int y = 0; y < 2 && y < sp->sprite_h; y++) {
			for (int x = 0; x < 4 && x < sp->sprite_w; x++) {
				int current_color = getpixel(current_sprite, x, y);
				int master_color = getpixel(sp->bmp, master_x + x, y);

				printf("  [%d,%d] - Current: %d, Master: %d %s\n",
					x, y, current_color, master_color,
					(current_color == master_color) ? "MATCH" : "MISMATCH");

				if (current_color != master_color) {
					mismatches++;
				}
			}
		}

		if (mismatches > 0) {
			printf("WARNING: %d mismatches found in committed sprite!\n", mismatches);
		}
		else {
			printf("Commit verification: SUCCESS\n");
		}
	}
	printf("=== END COMMIT VERIFICATION ===\n\n");
}

int file_save_gfx(void)
{
	if (GameDriverLoaded)
	{
		busy();

		printf("=== STARTING SAVE PROCESS ===\n");
		printf("Current graphics bank: %d\n", currentGfxBank);
		printf("Current sprite index: %d\n", current_sprite_index);

		// Verify the current sprite index is valid
		if (currentGfxBank >= 0 && currentGfxBank < NumGfxBanks) {
			SPRITE_PALETTE* sp = &GfxBanks[currentGfxBank];
			if (current_sprite_index >= sp->n_total) {
				printf("WARNING: current_sprite_index %d is out of range (0-%d)\n",
					current_sprite_index, sp->n_total - 1);
				current_sprite_index = 0; // Reset to safe value
			}
		}

		// Commit current edits to master bitmap
		printf("Committing current sprite to master bitmap...\n");
		CommitCurrentSpriteToMaster();

		// Verify the commit worked
		printf("Verifying commit...\n");
		VerifyCurrentSpriteCommit();

		// Encode ALL graphics banks to ROM data
		printf("Encoding all graphics banks to ROM data...\n");
		for (int bank = 0; bank < NumGfxBanks; bank++) {
			if (GfxBanks[bank].bmp) {
				printf("Encoding bank %d...\n", bank);
				Encode(bank);
			}
		}

		printf("Calling SaveDriver...\n");
		if (SaveDriver() == TRUE) {
			not_busy();
			alert("", "Save complete", NULL, "&Okay", NULL, 'O', 0);
			printf("=== SAVE COMPLETED SUCCESSFULLY ===\n");
		}
		else {
			printf("=== SAVE FAILED ===\n");
		}
		not_busy();
	}
	else {
		alert("Cannot save graphics!", "No romdata loaded!", "Sorry.",
			"&Okay", NULL, 'O', 0);
	}

	return D_O_K;
}

void file_save_c_source() {
	printf("=== DEBUG file_save_c_source: Starting ===\n");

	char path[256] = { 0 };

	// Generate filename based on game description
	if (GameDescription[0] != '\0') {
		// Use game description for filename (replace spaces with underscores)
		strncpy(path, GameDescription, sizeof(path) - 1);
		for (char* p = path; *p; p++) {
			if (*p == ' ') *p = '_';
			// Remove other unsafe characters for filenames
			if (*p == '/' || *p == '\\' || *p == ':' || *p == '*' ||
				*p == '?' || *p == '"' || *p == '<' || *p == '>' || *p == '|') {
				*p = '_';
			}
		}
		// Add .c extension
		size_t len = strlen(path);
		if (len < sizeof(path) - 3) {
			strcat(path, ".c");
		}
		else {
			// Truncate if too long
			path[sizeof(path) - 4] = '\0';
			strcat(path, ".c");
		}
	}
	else {
		strncpy(path, "sprite_data.c", sizeof(path) - 1);
	}

	printf("DEBUG: Saving C source to: %s\n", path);

	// Try to create the file
	FILE* fp = fopen(path, "w");
	if (!fp) {
		DWORD error = GetLastError();
		printf("ERROR: Cannot open file '%s' for writing, error: %lu\n", path, error);
		DisplayError("Cannot open file for writing: %s", path);
		return;
	}

	printf("DEBUG: File opened successfully, saving C source...\n");

	// Save sprite data as C source
	fprintf(fp, "/* C Source generated by Turaco */\n");
	fprintf(fp, "/* Game: %s */\n\n", GameDescription);
	fprintf(fp, "#include <stdint.h>\n\n");

	// Export palette data
	fprintf(fp, "/* Color Palettes */\n");
	for (int plane = 0; plane < MAX_COL_PLANES; plane++) {
		if (NumColPalettes[plane] > 0) {
			int num_colors = 1 << plane;
			fprintf(fp, "/* %d-color palettes (%d-bit) */\n", num_colors, plane);

			for (int pal_index = 0; pal_index < NumColPalettes[plane]; pal_index++) {
				fprintf(fp, "const uint8_t palette_%dbits_%d[%d][3] = {\n",
					plane, pal_index, num_colors);

				for (int color = 0; color < num_colors; color++) {
					int offset = num_colors * pal_index;
					fprintf(fp, "    { %3d, %3d, %3d }, /* Color %d */\n",
						ColPalettes[plane][offset + color].r,
						ColPalettes[plane][offset + color].g,
						ColPalettes[plane][offset + color].b,
						color);
				}
				fprintf(fp, "};\n\n");
			}
		}
	}

	// Export sprite data (basic example - you'll need to adapt this)
	if (GameDriverLoaded && NumGfxBanks > 0) {
		fprintf(fp, "/* Sprite Data */\n");
		for (int bank = 0; bank < NumGfxBanks; bank++) {
			fprintf(fp, "/* Bank %d: %dx%d sprites, %d total */\n",
				bank, GfxBanks[bank].sprite_w, GfxBanks[bank].sprite_h,
				GfxBanks[bank].n_total);

			// This is a simplified example - you'll need to implement 
			// the actual sprite data extraction based on your format
			fprintf(fp, "/* Sprite extraction code would go here */\n\n");
		}
	}
	else {
		fprintf(fp, "/* No sprite data available - no game loaded */\n");
	}

	fprintf(fp, "/* End of generated C source */\n");

	fclose(fp);
	printf("DEBUG: C source saved successfully to: %s\n", path);

	// Show success message
	char message[512];
	snprintf(message, sizeof(message), "C source saved to:\n%s", path);
	alert("Success", message, NULL, "OK", NULL, 'O', 0);

	printf("=== DEBUG file_save_c_source: Completed ===\n");
}

int file_genpatch(void)
{
    // generate the patch file from the data changed...
    if(GameDriverLoaded)
    {
	busy();

	// fix for bug 07 -- perhaps we should just Encode() here?
	SwitchGraphicsBank(currentGfxBank, currentGfxBank);

	MakePatch();
	not_busy();
	alert("", "Patch generation complete", NULL, "&Okay", NULL, 'O', 0);

    } else {
	alert("Cannot generate patch!", "No romdata loaded!", "Sorry.",
	"&Okay", NULL, 'O', 0);
    }
    return D_O_K;
}

int file_applypatch(void)
{
    // generate the patch file from the data changed...
    if(GameDriverLoaded)
    {
	busy();

	ApplyPatch();

	set_Gfx_bank(0);

	not_busy();

	alert("", "Patch application complete", NULL, "&Okay", NULL, 'O', 0);
	return D_REDRAW;
    } else {
	alert("Cannot apply patch!", "No romdata loaded!", "Sorry.",
	"&Okay", NULL, 'O', 0);
    }
    return D_O_K;
}