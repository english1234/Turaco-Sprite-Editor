// Editmenu.c
//
//  edit menu functions
//
//  September, 1998
//  jerry@mail.csh.rit.edu

#include "../INCLUDE/ALLEGRO.H"
#include <string.h>  // strncpy
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> 
#include <stdbool.h>
#include "../INCLUDE/general.h"
#include "../INCLUDE/config.h"
#include "../INCLUDE/coding.h" // commit graphics banks
#include "../INCLUDE/sprtplte.h"
#include "../INCLUDE/sprite.h"
#include "../INCLUDE/gamedesc.h"
#include "../INCLUDE/bmpdisp.h"
#include "../INCLUDE/GUIPAL.H"

#include SDL_PATH

MYBITMAP * clipboard = NULL;
extern MYBITMAP* screen;          // Main display bitmap
extern MYBITMAP* current_sprite;
extern FONT* font;              // Default system font
extern FONT my_new_font;
extern FONT* original_font;
extern int gui_mg_color;

extern MYBITMAP* create_bitmap(int width, int height);
extern void text_mode(int mode);
extern int alert(const char*, const char*, const char*, const char*, const char*, int, int);
extern int do_dialog(DIALOG* dialog, int focus_obj);
extern int button_dp2_proc(int msg, DIALOG* d, int c);

// Add these declarations with the other dialog procedures
extern int d_edit_proc(int msg, DIALOG* d, int c);
extern int d_check_proc(int msg, DIALOG* d, int c);  // If you implement it

// Also add any missing variable declarations
extern int drv_subdirs;
extern int troll_magic;
extern int gfx_hres, gfx_vres;
extern char ROMPath[ROM_PATH_LEN];  // Define ROM_PATH_LEN if not already defined

extern volatile int mouse_x;
extern volatile int mouse_y;

// UNDO / REDO

int edit_undo(void)
{
    return D_O_K;
}

int edit_redo(void)
{
    return D_O_K;
}


// COPY / PASTE (SNARF/BARF)

int edit_copy_sprite(void) {
	printf("Edit -> Copy Sprite executed!\n");

	if (current_sprite == NULL) {
		printf("No current sprite to copy\n");
		return D_O_K;
	}

	if (clipboard != NULL) {
		destroy_bitmap(clipboard);
		clipboard = NULL;
	}

	clipboard = create_bitmap(current_sprite->w, current_sprite->h);
	if (clipboard) {
		blit(current_sprite, clipboard, 0, 0, 0, 0, current_sprite->w, current_sprite->h);
		printf("Sprite copied to clipboard\n");
	}

	return D_REDRAW; // This will force a redraw
}

int edit_paste_sprite(void) {
	printf("Edit -> Paste Sprite executed!\n");

	if (current_sprite == NULL || clipboard == NULL) {
		printf("Cannot paste - no clipboard data or no current sprite\n");
		return D_O_K;
	}

	blit(clipboard, current_sprite, 0, 0, 0, 0, current_sprite->w, current_sprite->h);
	printf("Sprite pasted from clipboard\n");

	return D_REDRAW;
}

// RIP FUNCTIONS

char nsprites_wide[16];
char final_resolution[64];
int old = -1;
extern DIALOG save_pcx_dialog[];

int nd_edit_proc(int msg, DIALOG* d, int c)
{
	int new;
	if (msg == MSG_IDLE)
	{
		new = atoi(nsprites_wide);
		if (new != old)
		{
			old = new;

			if (new != 0)
			{
				sprintf(final_resolution, "%d x %d        ",
					(new * current_sprite->w),
					GfxBanks[currentGfxBank].n_total
					/ new * current_sprite->h);
			}
			else {
				sprintf(final_resolution, "?            ");
			}
			final_resolution[15] = '\0';

			show_mouse(NULL);
			save_pcx_dialog[7].proc(MSG_DRAW, &save_pcx_dialog[7], 0);
			show_mouse(screen);
		}

	}

	if (msg == MSG_CHAR)
	{
		if (isprint(c))
		{
			if (isdigit(c))
				return(d_edit_proc(msg, d, c));
			else
				return D_USED_CHAR;
		}
		else
			return(d_edit_proc(msg, d, c));

	}
	else
		return(d_edit_proc(msg, d, c));

	return D_O_K;
}


static DIALOG save_pcx_dialog[] =
{
	/* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp) */
	{ d_shadow_box_proc, 0,    0,    304,  160,  0,    0,    0,    0,       0,    0,    NULL },
	{ d_ctext_proc,      152,  8,    1,    1,    0,    0,    0,    0,       0,    0,    "Image save settings..." },
	{ d_button_proc,     208,  107,  80,   16,   0,    0,    'o',  D_EXIT,  0,    0,    "&OK" },
	{ d_button_proc,     208,  129,  80,   16,   0,    0,    'c',  D_EXIT,  0,    0,    "&Cancel" },

	{ d_text_proc,      40,  38,     1,   1,    0,    0,    0,    0,       0,    0,    "No. Sprites Wide:"},
	{ nd_edit_proc,     180, 38,     50,  8,    0,    0,    0,    0,       4,    0,    nsprites_wide },

	{ d_text_proc,      40,  60,     1,   1,    0,    0,    0,    0,       0,    0,    "Final Resolution:"},
	{ d_text_proc,      180, 60,     1,   1,    0,    0,    0,    0,       0,    0,    final_resolution},
	{ NULL }
};




#define SPD_OK     (2)
#define SPD_CANCEL (3)

SDL_Color* get_current_palette(void) {
	// This depends on how you're handling palettes in your SDL port
	// Return your current SDL palette array or NULL if not available
	return NULL; // Replace with actual implementation
}

int edit_save_pcx(void)
{
    char the_path[255];
    char newname[32];
    char numbre[32];
    char *pos;
    MYBITMAP * tb;
    MYBITMAP * save_me;
    RGB pal[256];
    int count;
    int x,y;
    int sx,sy;
    int wide;

    if (!GameDriverLoaded)
    {
	alert("Cannot save pcx!", "No romdata loaded!", "Sorry.", 
		"&Okay", NULL, 'O', 0);
	return D_REDRAW;
    }

    // get user preferences
    strncpy(nsprites_wide, get_config_string("PCX_Rip", "NumPerRow", "32"), 16);

    centre_dialog(save_pcx_dialog);
    set_dialog_color(save_pcx_dialog, GUI_FORE, GUI_BACK);
    if (do_dialog(save_pcx_dialog, 2) == SPD_OK)
    {
	set_config_string("PCX_Rip", "NumPerRow", nsprites_wide);
	wide = atoi(nsprites_wide);

	strncpy(the_path, get_config_string("PCX_Rip", "Path", "."), 255);
	put_backslash(the_path);

	// generate a possible name for the image...
	// in the form:  pacman bank 4:  pacm04.pcx
	strcpy(newname, ROMDirName);
	newname[6] = '\0';
	sprintf(numbre, "%02d", currentGfxBank+1);
	strcat(newname, numbre);
	strcat(the_path, newname);
	strcat(the_path, ".PCX");

	if (file_select("Enter A PCX Image filename:", the_path, "PCX") )
	{
	    Commit_Graphics_Bank();

	    // save it out now 
	    tb = create_bitmap(current_sprite->w, current_sprite->h);
	    sx = wide * (current_sprite->w);
	    sy = GfxBanks[currentGfxBank].n_total
		 / wide  * (current_sprite->h);

	    save_me = create_bitmap(sx, sy);

	    if (tb && save_me)
	    {
		// generate the bitmap to be saved
		x = y = 0;
		for (count = 0 ; 
		     count < GfxBanks[currentGfxBank].n_total ;
		     count ++)
		{
		    if (x >= wide)
		    {
			x = 0;
			y++;
		    }

		    // get a sprite
		    sprite_get(count, tb, sprite_bank);

		    // color shift it
		    for (sx=0 ; sx < tb->w ; sx++)
		        for (sy=0 ; sy < tb->h ; sy++)
			    putpixel(tb, sx, sy, 
			             getpixel(tb, sx, sy) - FIRST_USER_COLOR);
		    // copy it to the save bitmap
		    blit(tb, save_me, 0, 0,
			 x*tb->w, y*tb->h, tb->w, tb->h);

		    x++;
		}

		// generate the palette to be saved
		// generate the palette to be saved
		for (count = FIRST_USER_COLOR; count < 256; count++)
		{
			// Replace _current_palette with your SDL palette implementation
			// This is a common SDL palette approach:
			SDL_Color* current_palette = get_current_palette(); // You'll need to implement this
			if (current_palette) {
				pal[count - FIRST_USER_COLOR].r = current_palette[count].r;
				pal[count - FIRST_USER_COLOR].g = current_palette[count].g;
				pal[count - FIRST_USER_COLOR].b = current_palette[count].b;
			}
			else {
				// Fallback: use grayscale
				pal[count - FIRST_USER_COLOR].r = count;
				pal[count - FIRST_USER_COLOR].g = count;
				pal[count - FIRST_USER_COLOR].b = count;
			}
		}
		// let the last colors just be whatever they are -- they're 
		// not important anyway...

		// and finally ... save it out!
		save_pcx(the_path, save_me, pal);
	    }

	    if (tb)       destroy_bitmap(tb);
	    if (save_me)  destroy_bitmap(save_me);

	    pos = get_filename(the_path);
	    *pos = '\0';
	    set_config_string("PCX_Rip", "Path", the_path);

	}
    }
    return(D_REDRAW);
}

MYBITMAP * elpd_bmp = NULL;
BITMAP_DISPLAY_STRUCT * bds = NULL;

int home_callback(DIALOG * d, int ic)
{
    if (bds)
    {
	bds->x = 0;
	bds->y = 0;
	return (D_REDRAW);
    }
    return (D_O_K);
}

int grid_callback(DIALOG * d, int ic)
{
    return (D_REDRAW);
}


int elpd_callback(DIALOG * d, int x, int y, int mb)
{
//    blit(s,d, sx,sy, dx,dy, w,h)

    blit(elpd_bmp, current_sprite, x,y, 0,0, 
         current_sprite->w, current_sprite->h);

    return D_EXIT;
}

DIALOG edit_load_PCX_dialog[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  
   		(key) (flags)  (d1)  (d2)  (dp) */

    /// base dialog stuff 
    { d_shadow_box_proc, 0,    0,    314,  235,  0,    0,    
                0,    0,       0,    0,    NULL },
    { d_ctext_proc,      160,  7,    1,    1,    0,    0,    
                0,    0,       0,    0,   "Select Sprite Region"},

    { bitmap_display_proc,      5, 20, 304, 155, 0, 0, 
                0, 0, 0, 0, &elpd_bmp, &bds, elpd_callback },
    
    { button_dp2_proc,   5, 180, 80, 16, 0, 0,
   		'h', D_EXIT,  0, 0, "&Home",  home_callback },

    { button_dp2_proc,   225, 190, 80, 16, 0, 0,
   		'g', D_EXIT,  0, 0, "&Grid",  grid_callback },

    { d_button_proc,   225,  210,  80,   16,   0,    0,    
                'c',   D_EXIT,  0,    0,    "&Close", NULL },

    { NULL },
};
#define ELPD_BMP_DISPLAY (2)


void adjust_bitmap(MYBITMAP * bmp, int colors)
{
    int x, y, c;

    if (bmp == NULL)
	return;

    for (x=0 ; x< bmp->w ; x++)
	for (y=0 ; y< bmp->h ; y++)
	{
	    c = (getpixel(bmp, x, y))%colors;
	    c += FIRST_USER_COLOR;
	    putpixel(bmp, x, y, c);
	}

}

int edit_load_pcx(void)
{
    RGB a_palette[256];
    char the_path[255];
    char *pos;

    if (!GameDriverLoaded)
    {
	alert("Cannot oade pcx!", "No romdata loaded!", "Sorry.", 
		"&Okay", NULL, 'O', 0);
	return D_REDRAW;
    }

    strncpy(the_path, get_config_string("PCX_Rip", "Path", "."), 255);
    put_backslash(the_path);

    if (file_select("Select A PCX Image filename:", the_path, "PCX") )
    {
	elpd_bmp = load_pcx(the_path, a_palette);

	if (elpd_bmp)
	{
	    // perhaps in this place here, we should put in a selector
	    // to remap the colors... but for now, we will just modulo
	    // with the number of colors available in this bank, and add
	    // on FIRST_USER_COLOR
	    adjust_bitmap(elpd_bmp, 
	                  (1 << GfxBankExtraInfo[currentGfxBank].planes)
	                 );

	    /*
	    */ bds = NULL;
	    bds = (BITMAP_DISPLAY_STRUCT *) 
		      malloc(sizeof(BITMAP_DISPLAY_STRUCT));

	    bds->x = bds->y = 0;
	    bds->size = current_sprite->w;
	    bds->grid = FALSE;
	    bds->sel_w = current_sprite->w;
	    bds->sel_h = current_sprite->h;

	    centre_dialog(edit_load_PCX_dialog);
	    set_dialog_color(edit_load_PCX_dialog, GUI_FORE, GUI_BACK);
	    do_dialog(edit_load_PCX_dialog, -1);

	    free(bds);
	} else {
	    alert("", "Unable to load the bitmap", "", 
	          "&Bummin", NULL, 'b', 0);
	}

	pos = get_filename(the_path);
	*pos = '\0';
	set_config_string("PCX_Rip", "Path", the_path);
    }
    return(D_REDRAW);
}

// Quote from "The fairly decent three":
// How many of you are there?
//   um.... counting the blind man and the gimp...  three.
//   but you really can't count the gimp.  he can't pull a trigger.

// MISC PREFERENCES 
extern DIALOG edit_prefs_dialog[];

int do_settings(int msg, DIALOG *d, int c);

///// resolution stuff

int r320x240(void) { gfx_hres=320;  gfx_vres=240; return D_REDRAW; }
int r320x400(void) { gfx_hres=320;  gfx_vres=400; return D_REDRAW; }
int r320x480(void) { gfx_hres=320;  gfx_vres=480; return D_REDRAW; }
int r640x480(void) { gfx_hres=640;  gfx_vres=480; return D_REDRAW; }
int r800x600(void) { gfx_hres=800;  gfx_vres=600; return D_REDRAW; }
int r1024x768(void){ gfx_hres=1024; gfx_vres=768; return D_REDRAW; }

MENU resolution_popup[] =
{
    {"320x240",  r320x240, NULL, 0, NULL},
    {"320x400",  r320x400, NULL, 0, NULL},
    {"320x480",  r320x480, NULL, 0, NULL},
    {"640x480",  r640x480, NULL, 0, NULL},
    {"800x600",  r800x600, NULL, 0, NULL},
    {"1024x768", r1024x768, NULL, 0, NULL},
    {NULL}
};

///// end resolution stuff

// Forward declaration
int resolution_callback(DIALOG* d, int ic);

DIALOG edit_prefs_dialog[] =
{
	/* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)
		 (key) (flags)  (d1)  (d2)  (dp) */

		 /// base dialog stuff 
		 { d_shadow_box_proc, 0,    0,    314,  235,  0,    0,
					 0,    0,       0,    0,    NULL },
		 { d_ctext_proc,      160,  7,    1,    1,    0,    0,
					 0,    0,       0,    0,   "Turaco User Preferences"},

	 #define DE_USE_SUBDIRS (2)
{ d_check_proc,      6, 40, 200, 16, 0, 0,  // Reduced width from 290 to 200
		'd', 0, 0, 0, "Use &Driver Subdirectories" },

#define DE_NEW_FONT (3)
{ d_check_proc,    6, 54, 200, 16, 0, 0,    // Reduced width from 290 to 200
		'n', 0, 0, 0, "Use &New Font" },

#define DE_TROLL_MAGIC (4)
{ d_check_proc,    6, 68, 200, 16, 0, 0,    // Reduced width from 290 to 200
		't', 0, 0, 0, "&Troll Magic" },

		 { d_text_proc,      10,  90,    1,    1,    0,    0,
					 0,    0,       0,    0,   "Rom Paths:"},
		 { d_edit_proc,       10, 100, 285, 16, 0, 0,
					 0, 0, ROM_PATH_LEN, 1, ROMPath },

		 { d_text_proc,      10,  130,    1,    1,    0,    0,
					 0, 0,   0,    0,   "Screen Resolution:"},

	 #define DE_RESOLUTION (8)
					 // FIX: Use a modifiable buffer instead of string literal
					 { button_dp2_proc,   10, 140, 100, 16, GUI_FORE, GUI_BACK,
						 0, D_EXIT,  0, 0, NULL,
						 resolution_callback },

					 { d_button_proc,   225,  203,  80,   16,   0,    0,
								 'c',   D_EXIT,  0,    0,    "&Close", NULL },

					 { do_settings, 0, 0, 314, 0 },
					 { NULL, 0, 0, 0, 0, GUI_FORE, GUI_BACK,  0, 0, 0, 0, NULL }
};

int d_check_proc(int msg, DIALOG* d, int c) {
	static int initialized = 0;

	if (msg == MSG_START) {
		initialized = 1;
	}

	if (msg == MSG_DRAW) {
		// DON'T fill the entire background - let the dialog handle that
		// Only draw the checkbox elements

		int fg = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;

		// Draw checkbox square
		int checkbox_size = 10;
		int checkbox_x = d->x;
		int checkbox_y = d->y + (d->h - checkbox_size) / 2;

		// Draw checkbox background (just the square, not the whole area)
		rectfill(screen, checkbox_x, checkbox_y,
			checkbox_x + checkbox_size - 1, checkbox_y + checkbox_size - 1, d->bg);

		// Draw checkbox border
		rect(screen, checkbox_x, checkbox_y,
			checkbox_x + checkbox_size - 1, checkbox_y + checkbox_size - 1, fg);

		// Draw cross if selected
		if (d->flags & D_SELECTED) {
			line(screen, checkbox_x + 2, checkbox_y + 2,
				checkbox_x + checkbox_size - 2, checkbox_y + checkbox_size - 2, fg);
			line(screen, checkbox_x + checkbox_size - 2, checkbox_y + 2,
				checkbox_x + 2, checkbox_y + checkbox_size - 2, fg);
		}

		// Draw text
		if (d->dp) {
			text_mode(-1);

			// Parse text to remove '&' character and find underline position
			char* text = (char*)d->dp;
			char display_text[256];
			int display_pos = 0;
			int underline_pos = -1;

			for (int j = 0; text[j] != '\0' && display_pos < 255; j++) {
				if (text[j] == '&') {
					underline_pos = display_pos;
				}
				else {
					display_text[display_pos++] = text[j];
				}
			}
			display_text[display_pos] = '\0';

			int text_x = checkbox_x + checkbox_size + 4;
			int text_y = d->y + (d->h - text_height(font)) / 2;

			textout(screen, font, display_text, text_x, text_y, fg);

			// Draw underline under the shortcut key
			if (underline_pos >= 0) {
				int underline_x = text_x + (underline_pos * 8);
				int underline_y = text_y + 7;
				hline(screen, underline_x, underline_y, underline_x + 7, fg);
			}
		}
		return D_O_K;
	}

	// Handle all interaction types
	if (msg == MSG_CLICK || msg == MSG_LPRESS) {
		if (mouse_x >= d->x && mouse_x < d->x + d->w &&
			mouse_y >= d->y && mouse_y < d->y + d->h) {

			d->flags ^= D_SELECTED;

			// IMMEDIATELY UPDATE VARIABLES WHEN CHECKBOX IS TOGGLED
			if (d == &edit_prefs_dialog[DE_NEW_FONT]) {
				if (d->flags & D_SELECTED) {
					font = &my_new_font;
					printf("DEBUG: Checkbox toggled - setting font to my_new_font\n");
				}
				else {
					font = original_font;
					printf("DEBUG: Checkbox toggled - setting font to original_font\n");
				}
			}
			else if (d == &edit_prefs_dialog[DE_USE_SUBDIRS]) {
				drv_subdirs = (d->flags & D_SELECTED) ? 1 : 0;
				printf("DEBUG: Checkbox toggled - setting drv_subdirs to %d\n", drv_subdirs);
			}
			else if (d == &edit_prefs_dialog[DE_TROLL_MAGIC]) {
				troll_magic = (d->flags & D_SELECTED) ? 1 : 0;
				printf("DEBUG: Checkbox toggled - setting troll_magic to %d\n", troll_magic);
			}

			object_message(d, MSG_DRAW, 0); // Force redraw
			return D_REDRAWME;
		}
	}

	return D_O_K;
}

int resolution_callback(DIALOG* d, int ic)
{
	static char temp_buffer[32];
	int sel = do_menu(resolution_popup, d->x + 10, d->y + 4);

	if (sel >= 0) {
		strcpy(temp_buffer, resolution_popup[sel].text);
		edit_prefs_dialog[DE_RESOLUTION].dp = temp_buffer;
	}

	return(D_REDRAW);
}


int do_settings(int msg, DIALOG* d, int c)
{
	// Add a static buffer for the resolution string
	static char resolution_buffer[32] = "640x480"; // Default value

	if (msg == MSG_START)
	{
		printf("DEBUG: do_settings MSG_START - Initializing dialog\n");

		// Initialize checkboxes based on current global variables
		if (drv_subdirs) {
			edit_prefs_dialog[DE_USE_SUBDIRS].flags |= D_SELECTED;
			printf("DEBUG: drv_subdirs is ON\n");
		}
		else {
			edit_prefs_dialog[DE_USE_SUBDIRS].flags &= ~D_SELECTED;
			printf("DEBUG: drv_subdirs is OFF\n");
		}

		// FIX: Use the actual current font state, not just the variable
		if (font == &my_new_font) {
			edit_prefs_dialog[DE_NEW_FONT].flags |= D_SELECTED;
			printf("DEBUG: font is my_new_font (checkbox should be checked)\n");
		}
		else {
			edit_prefs_dialog[DE_NEW_FONT].flags &= ~D_SELECTED;
			printf("DEBUG: font is original_font (checkbox should be unchecked)\n");
		}

		// Update resolution display
		sprintf(resolution_buffer, "%dx%d", gfx_hres, gfx_vres);
		edit_prefs_dialog[DE_RESOLUTION].dp = resolution_buffer;

		d->d1 = gfx_hres;
		d->d2 = gfx_vres;

		if (troll_magic) {
			edit_prefs_dialog[DE_TROLL_MAGIC].flags |= D_SELECTED;
			printf("DEBUG: troll_magic is ON\n");
		}
		else {
			edit_prefs_dialog[DE_TROLL_MAGIC].flags &= ~D_SELECTED;
			printf("DEBUG: troll_magic is OFF\n");
		}

		return D_REDRAW;
	}

	if (msg == MSG_DRAW)
	{
		line(screen, d->x + 10, d->y + 20, d->x + d->w - 10, d->y + 20, GUI_FORE);
		line(screen, d->x + 10, d->y + 21, d->x + d->w - 10, d->y + 21, GUI_MID);
	}

	if (msg == MSG_END)
	{
		printf("DEBUG: do_settings MSG_END - Updating preferences\n");

		// Update global variables from dialog state
		if (edit_prefs_dialog[DE_TROLL_MAGIC].flags & D_SELECTED)
			troll_magic = 1;
		else
			troll_magic = 0;

		if (edit_prefs_dialog[DE_USE_SUBDIRS].flags & D_SELECTED)
			drv_subdirs = 1;
		else
			drv_subdirs = 0;

		// Font is already updated by checkbox handler

		printf("DEBUG: Final state - drv_subdirs=%d, troll_magic=%d, font=%s\n",
			drv_subdirs, troll_magic, (font == &my_new_font) ? "new" : "original");

		// DON'T save here - let Save_INI() handle it
		// Just update the resolution for the alert
		d->d1 = gfx_hres;
		d->d2 = gfx_vres;

		if (gfx_hres != d->d1 || gfx_vres != d->d2) {
			alert("The new screen resolution will",
				"be active the next time TURACO",
				"is restarted",
				"Okay", NULL, 0, 0);
		}

		// Call Save_INI to save all preferences
		Save_INI();
	}

	// Handle all interaction types
	if (msg == MSG_CLICK || msg == MSG_LPRESS) {
		if (mouse_x >= d->x && mouse_x < d->x + d->w &&
			mouse_y >= d->y && mouse_y < d->y + d->h) {
			printf("DEBUG: Checkbox clicked, toggling state. Old flags: 0x%X\n", d->flags);
			d->flags ^= D_SELECTED;
			printf("DEBUG: New flags: 0x%X\n", d->flags);

			// More specific checkbox identification
			printf("DEBUG: This checkbox text: '%s'\n", (char*)d->dp);
			printf("DEBUG: Font checkbox text: '%s'\n", (char*)edit_prefs_dialog[DE_NEW_FONT].dp);

			// Check by text content instead of pointer
			if (d->dp && strstr((char*)d->dp, "New Font")) {
				if (d->flags & D_SELECTED) {
					font = &my_new_font;
					printf("DEBUG: Font checkbox toggled - setting font to my_new_font\n");
					printf("DEBUG: Font pointer changed from %p to %p\n", original_font, &my_new_font);
				}
				else {
					font = original_font;
					printf("DEBUG: Font checkbox toggled - setting font to original_font\n");
					printf("DEBUG: Font pointer changed from %p to %p\n", &my_new_font, original_font);
				}
			}
			else if (d->dp && strstr((char*)d->dp, "Driver Subdirectories")) {
				drv_subdirs = (d->flags & D_SELECTED) ? 1 : 0;
				printf("DEBUG: drv_subdirs checkbox toggled - setting to %d\n", drv_subdirs);
			}
			else if (d->dp && strstr((char*)d->dp, "Troll Magic")) {
				troll_magic = (d->flags & D_SELECTED) ? 1 : 0;
				printf("DEBUG: troll_magic checkbox toggled - setting to %d\n", troll_magic);
			}
			else {
				printf("DEBUG: Unknown checkbox clicked: '%s'\n", (char*)d->dp);
			}

			object_message(d, MSG_DRAW, 0); // Force redraw
			return D_REDRAWME;
		}
	}

	return D_O_K;
}

void reset_prefs_dialog_state(void) {
	printf("DEBUG: Resetting dialog state\n");

	// Clear all checkbox flags first
	edit_prefs_dialog[DE_USE_SUBDIRS].flags &= ~D_SELECTED;
	edit_prefs_dialog[DE_NEW_FONT].flags &= ~D_SELECTED;
	edit_prefs_dialog[DE_TROLL_MAGIC].flags &= ~D_SELECTED;

	// Now set them based on current state
	if (drv_subdirs) {
		edit_prefs_dialog[DE_USE_SUBDIRS].flags |= D_SELECTED;
		printf("DEBUG: Setting drv_subdirs checkbox to SELECTED\n");
	}

	if (font == &my_new_font) {
		edit_prefs_dialog[DE_NEW_FONT].flags |= D_SELECTED;
		printf("DEBUG: Setting font checkbox to SELECTED\n");
	}
	else {
		printf("DEBUG: Setting font checkbox to UNSELECTED\n");
	}

	if (troll_magic) {
		edit_prefs_dialog[DE_TROLL_MAGIC].flags |= D_SELECTED;
		printf("DEBUG: Setting troll_magic checkbox to SELECTED\n");
	}

	// Verify the flags were set correctly
	printf("DEBUG: Font checkbox flags after reset: 0x%X (D_SELECTED=0x%X)\n",
		edit_prefs_dialog[DE_NEW_FONT].flags, D_SELECTED);
	printf("DEBUG: Font checkbox is %s\n",
		(edit_prefs_dialog[DE_NEW_FONT].flags & D_SELECTED) ? "CHECKED" : "UNCHECKED");
}

void sync_checkbox_states(void) {
	printf("DEBUG: Syncing checkbox states with current variables\n");
	printf("DEBUG: Current state - font=%s, drv_subdirs=%d, troll_magic=%d\n",
		(font == &my_new_font) ? "new" : "original", drv_subdirs, troll_magic);

	// Sync font checkbox
	if (font == &my_new_font) {
		edit_prefs_dialog[DE_NEW_FONT].flags |= D_SELECTED;
	}
	else {
		edit_prefs_dialog[DE_NEW_FONT].flags &= ~D_SELECTED;
	}

	// Sync other checkboxes
	if (drv_subdirs) {
		edit_prefs_dialog[DE_USE_SUBDIRS].flags |= D_SELECTED;
	}
	else {
		edit_prefs_dialog[DE_USE_SUBDIRS].flags &= ~D_SELECTED;
	}

	if (troll_magic) {
		edit_prefs_dialog[DE_TROLL_MAGIC].flags |= D_SELECTED;
	}
	else {
		edit_prefs_dialog[DE_TROLL_MAGIC].flags &= ~D_SELECTED;
	}
}

void verify_font_pointers(void) {
	printf("=== FONT POINTER VERIFICATION ===\n");
	printf("font pointer: %p\n", font);
	printf("my_new_font pointer: %p\n", &my_new_font);
	printf("original_font pointer: %p\n", original_font);
	printf("Are they equal?\n");
	printf("font == &my_new_font: %s\n", (font == &my_new_font) ? "YES" : "NO");
	printf("font == original_font: %s\n", (font == original_font) ? "YES" : "NO");
	printf("=================================\n");
}

int edit_preferences(void)
{
	MYBITMAP* bmp = create_bitmap(current_sprite->w, current_sprite->h);
	if (bmp) {
		blit(current_sprite, bmp, 0, 0, 0, 0, current_sprite->w, current_sprite->h);
	}

	printf("DEBUG: Before opening preferences dialog\n");
	verify_font_pointers();

	// Sync checkbox states with current variables
	sync_checkbox_states();

	centre_dialog(edit_prefs_dialog);
	set_dialog_color(edit_prefs_dialog, GUI_FORE, GUI_BACK);

	do_dialog(edit_prefs_dialog, 14);

	printf("DEBUG: After closing preferences dialog\n");
	verify_font_pointers();

	// Save preferences after dialog closes
	Save_INI();

	if (bmp) {
		blit(bmp, current_sprite, 0, 0, 0, 0, current_sprite->w, current_sprite->h);
		destroy_bitmap(bmp);
	}

	return D_REDRAW;
}
