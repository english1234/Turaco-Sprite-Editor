// DLG.c
//
//  the main dialog file
//
//   Main editor dialog, menus & valve functions 
//
//  September, 1998
//  jerry@mail.csh.rit.edu

#include "../INCLUDE/ALLEGRO.H"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "../INCLUDE/general.h"
#include "../INCLUDE/snap.h"
#include "../INCLUDE/util.h"
#include "../INCLUDE/bitmaped.h"
#include "../INCLUDE/animview.h"
#include "../INCLUDE/guipal.h"
#include "../INCLUDE/sprtplte.h"
#include "../INCLUDE/palette.h"
#include "../INCLUDE/sprite.h"
#include "../INCLUDE/editmenu.h"
#include "../INCLUDE/toolmenu.h"
#include "../INCLUDE/editmode.h"
#include "../INCLUDE/inidriv.h"
#include "../INCLUDE/gamedesc.h"
#include "../INCLUDE/fileio.h"
#include "../INCLUDE/maped.h"
#include "../INCLUDE/texted.h"
#include "../INCLUDE/help.h"
#include "../INCLUDE/drivsel.h" // for driver dirs
#include "../INCLUDE/config.h" // for driver dirs
#include "../INCLUDE/life.h"
#include "../INCLUDE/animate.h"

extern MYBITMAP *current_sprite;

extern MYBITMAP* screen;          // Main display bitmap
extern FONT* font;              // Default system font
extern volatile int mouse_x;
extern volatile int mouse_y;

extern MYBITMAP* create_bitmap(int width, int height);
extern void text_mode(int mode);
extern int d_menu_proc(int msg, DIALOG* d, int c);
extern int button_dp2_proc(int msg, DIALOG* d, int c);

// Forward declaration
extern DIALOG main_dialog[];

// extern SPRITE_PALETTE * GfxBanks;
// extern int NumGfxBanks;
//SPRITE_PALETTE * sprite_bank = NULL; // the sprite being used in the display


int stub(void) { return D_O_K; }


//
///  The Menus...
//

const MENU file_menu[] =
{
    {"&Change Game", 	file_game, NULL, 0, NULL},
    {"", 		NULL, NULL, 0, NULL},
    {"&Save Graphics" ,	file_save_gfx, NULL, 0, NULL},
    {"&Revert ",	file_revert, NULL, 0, NULL},
    {"", 		NULL, NULL, 0, NULL},
    {"&Generate Patch", file_genpatch, NULL, 0, NULL},
    {"&Apply Patch",	file_applypatch, NULL, 0, NULL},
    {"", 		NULL, NULL, 0, NULL},
    {"Sav&e C Source" ,	file_save_c_source, NULL, 0, NULL},
    {"", 		NULL, NULL, 0, NULL},
    {"E&xit", 		file_exit, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL},
};

const MENU edit_menu[] =
{
//    {"&Undo", 		edit_undo, NULL, D_DISABLED, NULL},
//    {"&Redo", 		edit_redo, NULL, D_DISABLED, NULL},
//    {"", 		NULL, NULL, 0, NULL},
    {"&Copy sprite", 	edit_copy_sprite, NULL, 0, NULL},
    {"&Paste sprite", 	edit_paste_sprite, NULL, 0, NULL},
    {"", 		NULL, NULL, 0, NULL},
    {"&Save Bank PCX", 	edit_save_pcx, NULL, 0, NULL},
    {"&Load From PCX", 	edit_load_pcx, NULL, 0, NULL},
    {"", 		NULL, NULL, 0, NULL},
//    {"&Map Editor", 	editors_map, NULL, D_DISABLED, NULL},
//    {"&Text Editor", 	editors_text, NULL, D_DISABLED, NULL},
//    {"", 		NULL, NULL, 0, NULL},
    {"&Add Palette", 	palette_add_new, NULL, 0, NULL},
    {"&Edit Palette", 	palette_edit, NULL, 0, NULL},
    {"", 		NULL, NULL, 0, NULL},
    {"Pre&ferences", 	edit_preferences, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL},
};

const MENU mode_menu[] =
{
    {"&p Paint",         edit_mode_paint, NULL, 0, NULL},
    {"&f Flood Fill",    edit_mode_flood, NULL, 0, NULL},
    {"&e Eyedrop", 	 edit_mode_eye, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL},
};


const MENU help_menu[] =
{
    {"&Readme",	      help_general, NULL, 0, NULL},
    {"&What's New",   help_new, NULL, 0, NULL},
    {"&About...",     help_about, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL},
};

const MENU main_menus[] =
{
    {"&File", 		NULL, file_menu,  0, NULL},
    {"&Edit", 		NULL, edit_menu,  0, NULL},
    {"&Mode",		NULL, mode_menu,  0, NULL},
    {"&Sprite",		NULL, tools_menu, 0, NULL},
    {"&Help", 		NULL, help_menu,  0, NULL},
    {NULL, NULL, NULL, 0, NULL},
};



////////////////////////////////////////////////////////////////////////////////
//
// "We're sorry we misinterpreted your running away as an attack."
//
////////////////////////////////////////////////////////////////////////////////
void DisplayGameDescription(int x, int y)
{
    char TruncatedName[50];

    // does text fit in area provided - if not then truncate it and add elipses
    if (strlen(GameDescription) > 22)
    {
	strncpy(TruncatedName, GameDescription, 20);
	TruncatedName[20] = '\0';
	strcat(TruncatedName, "...");
	textout(screen, font, TruncatedName, x, y, GUI_FORE);
    } else {
	// otherwise just display it.
	textout(screen, font, GameDescription, x, y, GUI_FORE);
    }
    // thanks ivan.. i missed that one.  :]
}

extern int timer_ticks;
int last_timer = 0;


int screen_text(int msg, DIALOG *d, int c)
{
    static int mouse_over_old = -1;
    static int cursor_over_old = -1;

    if (msg == MSG_IDLE)
    {
	// for lack of a better place to put this for now... 
	// don't like this? eat it.
	if (timer_ticks != last_timer)
	{
	    last_timer = timer_ticks;
	    animate_step();
	}

	if (GameDriverLoaded && NumGfxBanks)
	    if (GfxBanks[currentGfxBank].mouse_over != mouse_over_old
	       || GfxBanks[currentGfxBank].last_selected != cursor_over_old)
	    {
		cursor_over_old = GfxBanks[currentGfxBank].last_selected;
		mouse_over_old = GfxBanks[currentGfxBank].mouse_over;
		show_mouse(NULL);
		(void)screen_text(MSG_DRAW, d, c);

		main_dialog[11].proc(MSG_DRAW, &main_dialog[11], c);

		show_mouse(screen);
	    }

    }

    if (msg == MSG_DRAW)
    {
	// draw up all of the text onto the screen...
	text_mode(GUI_BACK);

	// number of banks
	if(GameDriverLoaded && NumGfxBanks)
	    textprintf_centre(screen, font, 265, 129, GUI_FORE, 
				"%02d/%02d", currentGfxBank+1, NumGfxBanks);
	else
	    textout_centre(screen, font, "--/--", 265, 129, GUI_FORE);

	border_3d(screen, 240, 125, 50, 15, 0);

        // number of palettes
        if (GameDriverLoaded && GfxBankExtraInfo)
        {
	    textprintf_centre(screen, font, 189, 74, GUI_FORE,
				"%d", current_palette_number +1);
	    textprintf_centre(screen, font, 189, 94, GUI_FORE, "%d", 
		NumColPalettes[GfxBankExtraInfo[currentGfxBank].planes] );
	} else {
	    textout_centre(screen, font, "--", 189, 74, GUI_FORE);
	    textout_centre(screen, font, "--", 189, 94, GUI_FORE);
	}
	line(screen, 181, 87, 196, 87, GUI_FORE);
	border_3d(screen, 179, 70, 20, 34, 0);

	// current sprite number
	if (GameDriverLoaded && NumGfxBanks)
	{
	    if (GfxBanks[currentGfxBank].mouse_over < 
		    GfxBanks[currentGfxBank].n_total)
	        textprintf_centre(screen, font, 290, 175, GUI_FORE,
			     "[0x%03x]",GfxBanks[currentGfxBank].mouse_over);
	    else if (GfxBanks[currentGfxBank].last_selected >= 0)
	        textprintf_centre(screen, font, 290, 175, GUI_FORE,
			     " 0x%03x ",GfxBanks[currentGfxBank].last_selected);
	    else
		textout_centre(screen, font, " 0x??? ", 290, 175, GUI_FORE);

	    textprintf_centre(screen, font, 290, 195, GUI_FORE,
			 "0x%03x",GfxBanks[currentGfxBank].n_total-1);

	} else {
	    textout_centre(screen, font, "0x---", 290, 175, GUI_FORE);
	    textout_centre(screen, font, "0x---", 290, 195, GUI_FORE);
	}
	line(screen, 269, 187, 310, 187, GUI_FORE);


	if (strlen(GameDescription))
	    DisplayGameDescription(4, 129);
	else
	    textout(screen, font, "No game loaded.", 4, 129, GUI_FORE);
	border_3d(screen, 0, 125, 205, 15, 0);
    }

    return D_O_K;
}

void try_loading_cmd_line_driver(void)
{
    char tstring[255];
    char * filename;
    unsigned int pos;
    int old_drv_subdirs;

    // try loading the game as described on the command line 
    // if there was one
    if (strlen(command_line_driver))
    {
        // strip the path
        filename = get_filename(command_line_driver);

        // strip the extension
        for (pos=0 ; pos < strlen(filename) ; pos++)
	    if (filename[pos] == '.') filename[pos] = '\0';
       
	sprintf(tstring, "expdriv\\%s.ini", filename);

	if (exists(tstring))
	{
	    try_loading_the_driver(tstring);
	} else {
	    sprintf(tstring, "drivers\\%s.ini", filename);
	    if (exists(tstring))
	    {
		try_loading_the_driver(tstring);
            } else {
		// now we'll try to check other subdirectories, 
		// without checking the ini file settings for subdirs.
		// that switch is just for the file selector...
                old_drv_subdirs = drv_subdirs;
                drv_subdirs = 1;
		create_dir_list();
		for (pos = 0 ; (int)pos < ndirs ; pos++)
		{
		    sprintf(tstring, "DRIVERS\\%s\\%s.ini", dirlist[pos], filename);
		    if (exists(tstring))
		    {
			try_loading_the_driver(tstring);
			break;
		    }
		}
		destroy_dir_list();
                drv_subdirs = old_drv_subdirs;
            }
	}
    }
}


int oneshot(int msg, DIALOG *d, int c)
{
    static BOOL first_time = TRUE; // need it to be static...

    // this is a kludge and a half, but we want this to be done
    // the first time available once the dialog has been drawn for 
    // the first time.

    // That means that we need to do it at the first idle time...

    if (msg == MSG_IDLE)
    {
        if (first_time)
        {
	    first_time = FALSE;

            // put all the stuff that needs to be done only once in here...

	    /*
	    alert("EXPERIMENTAL VERSION!", "USE AT YOUR OWN RISK!", "", 
		  "Got it!", NULL, 0, 0);
	    */

	    busy();
	    try_loading_cmd_line_driver();
	    not_busy();
	    if (GameDriverLoaded)
		return D_REDRAW;
	}
    }

    return D_O_K;
}

int my_kyb(int msg, DIALOG *d, int c)
{

    if (msg == MSG_START)
    {
	// create a dummy bitmap for the display...
	current_sprite = create_bitmap(ED_DEF_SIZE, ED_DEF_SIZE);
	if (current_sprite) {
		// Initialize with background color from palette system
		clear_to_color(current_sprite, Get_BG_Color());
	}
	/*
	line(current_sprite, 0, 0, 
	     current_sprite->w-1, current_sprite->h-1, 
	     FIRST_USER_COLOR+1);
	line(current_sprite, 0, 
	     current_sprite->h-1, current_sprite->w-1, 0, 
	     FIRST_USER_COLOR+1);
	*/

    }


    if (msg == MSG_END)
    {
	destroy_bitmap(current_sprite);
    }

    if (msg == MSG_XCHAR)
    {
	// sponges
	if ((c&0xff) == 27) { // escape
	    return D_USED_CHAR;
	} 

	if((SCANCODE_TO_KEY(KEY_F5)) == c)
	    return editors_map();

	if((SCANCODE_TO_KEY(KEY_F6)) == c)
	    return editors_text();

	if((SCANCODE_TO_KEY(KEY_F7)) == c)
	    return animate_force();

	if((SCANCODE_TO_KEY(KEY_F8)) == c)
	    return animate_toggle();

	if((SCANCODE_TO_KEY(KEY_F9)) == c)
	    return edit_save_pcx();

	if((SCANCODE_TO_KEY(KEY_F10)) == c)
	    return edit_load_pcx();

	if((SCANCODE_TO_KEY(KEY_F11)) == c)
	{
	    life_counter=0l;

	    show_mouse(NULL);
	    text_mode(GUI_BACK);
	    textprintf(screen, font, 261, 4, GUI_FORE, 
		       "%07ld ", life_counter);
	    show_mouse(screen);
	}

	if((SCANCODE_TO_KEY(KEY_F12)) == c)
	{
	    if (do_life() == D_REDRAW)
	    {
		show_mouse(NULL);
		main_dialog[2].proc(MSG_DRAW, &main_dialog[2], 0);

		text_mode(GUI_BACK);
		textprintf(screen, font, 261, 4, GUI_FORE, 
			   "%07ld ", life_counter);

		show_mouse(screen);
	    }
	}

	if((SCANCODE_TO_KEY(KEY_PGDN)) == c)
	    return sprite_pal_minus(NULL);

	if((SCANCODE_TO_KEY(KEY_PGUP)) == c)
	    return sprite_pal_plus(NULL);

	if((SCANCODE_TO_KEY(KEY_HOME)) == c)
	    return sprite_pal_home(NULL);

	// other keys
	if (c>>8 == KEY_P) { // alt-p
	    screen_snap();
	}

	if (c>>8 == KEY_A)  // alt-a
	    return animate_set_point();

	if (c>>8 == KEY_Z)  // alt-z
	    return animate_change_playmode();

    }
    return D_O_K;
}

////////////////////////////////////////////////////////////////////////////////
//
// "The cops in this adventure hang out at Dungeon Doughnuts."
//
////////////////////////////////////////////////////////////////////////////////

int slidcall(void *dp3, int d2)
{
    char te[64];

    sprintf(te, "%3d", d2);
    textout(screen, font, te, 200, 100, 2);
    return D_O_K;
}

// Add these global variable declarations if they don't exist
extern int pal_fg_color;
extern int pal_bg_color;
extern int edit_mode;

const char* edit_mode_to_string(int mode) {
	switch (mode) {
	case MODE_PAINT: return "MODE_PAINT";
	case MODE_FLOODFILL: return "MODE_FLOODFILL";
	case MODE_EYEDROP: return "MODE_EYEDROP";
	default: return "UNKNOWN";
	}
}

void bmed_callback(DIALOG* d, int x, int y, int mb)
{
	MYBITMAP** temp1;
	MYBITMAP* temp2;

	printf("=== BMED_CALLBACK CALLED ===\n");
	printf("Raw parameters - x: %d, y: %d, mb: %d\n", x, y, mb);

	// Ensure we have a valid sprite
	if (!d || !d->dp) {
		printf("ERROR: No dialog or bitmap pointer!\n");
		return;
	}

	temp1 = d->dp;  // pointer to the bitmap to draw
	temp2 = *temp1; // this is the bitmap to draw

	if (temp2 == NULL) {
		printf("ERROR: Bitmap is NULL! Creating default sprite.\n");
		initialize_default_sprite();
		temp2 = current_sprite;
		if (!temp2) {
			printf("CRITICAL: Could not create default sprite!\n");
			return;
		}
	}

	switch (edit_mode) {
	case(MODE_PAINT):
		printf("MODE_PAINT: ");
		if (mb & 1) {  // Left button (bit 0)
			printf("Left button - using FG color %d\n", pal_bg_color);
			do_tool(TOOLS_PAINT_FG, temp2, x, y);
		}
		else if (mb & 2) {  // Right button (bit 1)
			printf("Right button - using BG color %d\n", pal_fg_color);
			do_tool(TOOLS_PAINT_BG, temp2, x, y);
		}
		else {
			printf("No mouse button detected!\n");
		}
		break;

	case(MODE_FLOODFILL):
		printf("MODE_FLOODFILL: ");
		if (mb & 1) {
			printf("Left button - using FG color %d\n", pal_bg_color);
			do_tool(TOOLS_FLOOD_FILL_FG, temp2, x, y);
		}
		else if (mb & 2) {
			printf("Right button - using BG color %d\n", pal_fg_color);
			do_tool(TOOLS_FLOOD_FILL_BG, temp2, x, y);
		}
		break;

	case(MODE_EYEDROP):
		printf("MODE_EYEDROP: ");
		if (mb & 1) {
			printf("Left button - setting FG color\n");
			do_tool(TOOLS_EYEDROPPER_FG, temp2, x, y);
		}
		else if (mb & 2) {
			printf("Right button - setting BG color\n");
			do_tool(TOOLS_EYEDROPPER_BG, temp2, x, y);
		}
		set_mouse_edit_mode(edit_mode_old);
		break;
	}

	printf("=== END BMED_CALLBACK ===\n");
}

////////////////////////////////////////////////////////////////////////////////
//
// "It's hot."  "Not for pizza."  "No. Not for pizza."
//
////////////////////////////////////////////////////////////////////////////////

// d1 - range, d2-value , dp - bitmap, dp2 -callback
DIALOG main_dialog[] = {
	// Background clear
	{ d_clear_proc, 0, 0, 320, 240, GUI_FORE, GUI_BACK, 0, 0, 0, 0, NULL },

	// Menu bar (spans top)
	{ d_menu_proc, 0, 0, 320, 16, GUI_FORE, GUI_BACK, 0, 0, 0, 0, main_menus },

	// Bitmap editor (84x84 at 3.2x scale = 269x269)
	{ bitmap_editor_proc, 6, 19, 84, 84, GUI_FORE, GUI_BACK, 0, 0, 95, 19, &current_sprite, bmed_callback },

	// Edit/Back buttons
	{ button_dp2_proc, 6, 105, 41, 16, GUI_FORE, GUI_BACK, 0, D_EXIT, 0, 0, "Edit", sprite_capture },
	{ button_dp2_proc, 48, 107, 42, 16, GUI_FORE, GUI_BACK, 0, D_EXIT, 0, 0, "Back", sprite_return },

	// Palette display
	{ pal_display_proc, 93, 65, 20, 60, GUI_FORE, GUI_BACK, 0, 0, 0, 0, &pal_fg_color, &pal_bg_color},
	{ pal_select_proc, 115, 55, 60, 60, GUI_FORE, GUI_BACK, 0, 0, 0, 0, &psel, psel_callback },
	{ button_dp2_proc, 177, 55, 12, 9, GUI_FORE, GUI_BACK, 0, D_EXIT, 0, 0, "+", pal_plus },
	{ button_dp2_proc, 177, 106, 12, 9, GUI_FORE, GUI_BACK, 0, D_EXIT, 0, 0, "-", pal_minus },

	// Bank selection buttons
	{ button_dp2_proc, 210, 125, 30, 15, GUI_FORE, GUI_BACK, 0, D_EXIT, 0, 0, "<<", sprite_bank_minus },
	{ button_dp2_proc, 290, 125, 30, 15, GUI_FORE, GUI_BACK, 0, D_EXIT, 0, 0, ">>", sprite_bank_plus },

	// Screen text display (game name, sprite info, etc.)
	{ screen_text, 0, 0, 320, 240, GUI_FORE, GUI_BACK, 0, 0, 0, 0, NULL },

	// Sprite palette (260x100 at bottom)
	{ sprite_palette_proc, 0, 140, 260, 100, GUI_FORE, GUI_BACK, 0, 0, 0, 0, &sprite_bank, main_sprtplte_callback },
	{ button_dp2_proc, 260, 140, 9, 9, GUI_FORE, GUI_BACK, 0, D_EXIT, 0, 0, "^", sprite_pal_plus },
	{ button_dp2_proc, 260, 231, 9, 9, GUI_FORE, GUI_BACK, 0, D_EXIT, 0, 0, "v", sprite_pal_minus },

	// Animation view
	{ animview_proc, 236, 19, 84, 84, GUI_FORE, GUI_BACK, 0, 0, -1, 0},
	{ button_dp2_proc, 226, 19, 9, 9, GUI_FORE, GUI_BACK, 0, D_EXIT, 0, 0, "P", animate_change_playmode },
	{ button_dp2_proc, 226, 29, 9, 9, GUI_FORE, GUI_BACK, 0, D_EXIT, 0, 0, "A", animate_set_point },

	// One-shot initialization
	{ oneshot, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL },

	// Keyboard handler
	{ my_kyb, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL },

	// Terminator
	{ NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL }
};
