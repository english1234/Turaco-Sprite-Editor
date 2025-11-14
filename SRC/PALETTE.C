// palette.c
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

int pal_fg_color = FIRST_USER_COLOR;
int pal_bg_color = FIRST_USER_COLOR+1;

PALETTE_SELECT * psel = NULL; // the internal palette (for the gui)

BOOL pal_initted = FALSE;
int current_palette_number = 0;
/// max: NumColPalettes[GfxBankExtraInfo[currentGfxBank].planes]

int edit_color;

extern MYBITMAP* screen;          // Main display bitmap
extern FONT* font;              // Default system font
extern volatile int mouse_b;    // Mouse button state

extern MYBITMAP* create_bitmap(int width, int height);
extern void text_mode(int mode);
extern int alert(const char*, const char*, const char*, const char*, const char*, int, int);
extern int do_dialog(DIALOG* dialog, int focus_obj);
extern int button_dp2_proc(int msg, DIALOG* d, int c);

void Set_BG_Color(int color)
{
    pal_bg_color = color;
}

void Set_FG_Color(int color)
{
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

int psel_callback(DIALOG *d, int color, int mouse)
{
    int lc;

    // now set the internal 
    if(mouse & MOUSE_FLAG_LEFT_DOWN) { 
        // for some reason, these are inverted...
        //  <shrug?>
	lc = Get_BG_Color();
	if (color != lc) {
	    Set_BG_Color(color);
	}
    } else {
	lc = Get_FG_Color();
	if (color != lc) {
	    Set_FG_Color(color);
//	    return D_REDRAW;
	}
    }
    return D_O_K;
}


void install_fake_palette(void)
{
    // setup the palette colors such that it looks pretty.
    // ...or at least different...

    psel->ncolors = 2;

    psel->palette[FIRST_USER_COLOR].r = 20;
    psel->palette[FIRST_USER_COLOR].g = 20;
    psel->palette[FIRST_USER_COLOR].b = 30;

    psel->palette[FIRST_USER_COLOR+1].r = 50;
    psel->palette[FIRST_USER_COLOR+1].g = 30;
    psel->palette[FIRST_USER_COLOR+1].b = 30;

    Set_BG_Color(FIRST_USER_COLOR);
    Set_FG_Color(FIRST_USER_COLOR +1);
}

void install_palette(int pal_no) // installs the selected palette number
{
    int i;
    int PaletteIndex;
    int NumCols;

    // a few pointer checks
    if (GameDriverLoaded == FALSE)  
    {
	install_fake_palette();
	return;
    }

    if (GfxBankExtraInfo == NULL)
    {
	install_fake_palette();
	return;
    }

    if (psel == NULL)
    {
	install_fake_palette();
	return;
    }

    // the number of colour planes are used for indexing into the array of
    // colour palettes.
    PaletteIndex = GfxBankExtraInfo[currentGfxBank].planes;

    NumCols = 1 << GfxBankExtraInfo[currentGfxBank].planes;

    current_palette_number = pal_no;
    psel->ncolors = NumCols;

    // set appropriate amount of colours for this gfx bank
    for (i = 0; i < NumCols; i ++)
    {
        psel->palette[FIRST_USER_COLOR+i].r 
		= ColPalettes[PaletteIndex][NumCols*pal_no+i].r;
        psel->palette[FIRST_USER_COLOR+i].g 
		= ColPalettes[PaletteIndex][NumCols*pal_no+i].g;
        psel->palette[FIRST_USER_COLOR+i].b 
        	= ColPalettes[PaletteIndex][NumCols*pal_no+i].b;
    }

}

void DeInit_Palette(void)
{
    if (psel != NULL)
    {
	if (psel->palette)
	    free(psel->palette);
	free(psel);
    }
    psel = NULL; 

    pal_initted = FALSE;
}

void Init_Palette(void)
{
    if (pal_initted || psel != NULL)  DeInit_Palette();
    pal_initted = TRUE;

    psel = (PALETTE_SELECT *)malloc (sizeof (PALETTE_SELECT));
    psel->palette = (RGB *) malloc (sizeof (RGB) * 255); // may as well set this
    psel->basecolor = FIRST_USER_COLOR;
    psel->ncolors = 0;
    install_palette(0);
    Set_BG_Color(FIRST_USER_COLOR);
    Set_FG_Color(FIRST_USER_COLOR +1);
}

int pal_plus(DIALOG *d)
{
    if (GameDriverLoaded == FALSE)  return D_O_K;

    if(current_palette_number <
           NumColPalettes[GfxBankExtraInfo[currentGfxBank].planes]-1)
    {
	current_palette_number++;
	install_palette(current_palette_number);

	return D_REDRAW;
    } else 
        return D_O_K;
}

int palette_inc(void)
{ 
    return pal_plus(NULL);
}


int pal_minus(DIALOG *d)
{
    if (GameDriverLoaded == FALSE)  return D_O_K;

    if(current_palette_number)
    {
	current_palette_number--;
	install_palette(current_palette_number);

	return D_REDRAW;
    } else 
        return D_O_K;
}

int palette_dec(void)
{ 
    return pal_minus(NULL);
}


void palette_reset(void)
{
    if (GameDriverLoaded == FALSE)  return;
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
   int NumCols;
   int PalSize;
   int PalIndex;
   int j;
   RGB * tPal;

    // do a check before we do anything here
    if (GameDriverLoaded == FALSE)  return -1;

    NumCols = 1 << GfxBankExtraInfo[currentGfxBank].planes;
    PalSize = NumCols * sizeof(RGB);
    PalIndex = GfxBankExtraInfo[currentGfxBank].planes;

   // create memory for larger palette
   tPal = malloc(PalSize * (NumColPalettes[PalIndex] + 1));
   if (tPal == NULL)
      return -1;

   // copy old palette to new memory
   memcpy(tPal, ColPalettes[PalIndex], PalSize * NumColPalettes[PalIndex]);
   NumColPalettes[PalIndex] ++;

   // free old memory and copy pointer to new palette
   free(ColPalettes[PalIndex]);
   ColPalettes[PalIndex] = tPal;

   // copy current palette over the new one
   for (j = 0; j < NumCols; j ++)
   {
      int NewOffset = NumCols * (NumColPalettes[PalIndex] - 1);
      int CurrentOffset = NumCols * current_palette_number;
      
      ColPalettes[PalIndex][j + NewOffset].r 
	      = ColPalettes[PalIndex][j + CurrentOffset].r;
      ColPalettes[PalIndex][j + NewOffset].g 
	      = ColPalettes[PalIndex][j + CurrentOffset].g;
      ColPalettes[PalIndex][j + NewOffset].b 
	      = ColPalettes[PalIndex][j + CurrentOffset].b;
   }

   // save the new palette to the ini file
   SaveAllPalettes();
   
   // return the new palette number
   return NumColPalettes[PalIndex] -1;
}


int palette_add_new(void)
{
   int NewPaletteNum;

    if (GameDriverLoaded == FALSE)  return D_O_K;
   
   // reallocate the memory to make space for the new palette
   NewPaletteNum = CreateNewPaletteWithCurrentColours();
   if (NewPaletteNum == -1)
   {
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
    // install the palette...
    set_palette_range(psel->palette, psel->basecolor, 
                      psel->basecolor+psel->ncolors, 1);
    
    // redraw text on the screen
    show_mouse(NULL);
    palette_editor[9].proc(MSG_DRAW, &palette_editor[9], 0);
    show_mouse(screen);
}


int pal_edit_image_grab(DIALOG *d)
{
    char pal_path[255];
    char *pos =NULL;
    MYBITMAP *the_bitmap;
    RGB pall[256];
    int NumCols = 1 << GfxBankExtraInfo[currentGfxBank].planes;
    int i;

    for (i=0 ; i<255 ; i++)
	pall[i].r = pall[i].g = pall[i].b = (i&1)?0x42:0;

    strncpy(pal_path, get_config_string("System", "Pal_Grab_Path", "."), 255);
    put_backslash(pal_path);

    if (file_select("Choose an image...", pal_path, "BMP;LBM;PCX;TGA") )
    {
	busy();
	the_bitmap = load_bitmap(pal_path, pall);
	if (the_bitmap)
	{
	    destroy_bitmap(the_bitmap); // eh, we just want the palette.

	    // lets assume that the first N colors are the correct ones for now.

	    for (i = 0; i < NumCols; i ++)
	    {
		psel->palette[FIRST_USER_COLOR+i].r = pall[i].r;
		psel->palette[FIRST_USER_COLOR+i].g = pall[i].g;
		psel->palette[FIRST_USER_COLOR+i].b = pall[i].b;
	    }
	    refresh_palette_settings();
	    adjust_sliders();
	} else {
	    not_busy();

	    alert("Unable to load image file", pos, NULL, 
	          "&Bummer", NULL, 'O', 0);
	}

	pos = get_filename(pal_path);
	*pos = '\0';
	set_config_string("System", "Pal_Grab_Path", pal_path);
    }
    not_busy();

    return(D_REDRAW);
}


int pal_edit_cancel(DIALOG *d)
{
    // restore from before changes...
    int NumCols = 1 << GfxBankExtraInfo[currentGfxBank].planes;
    int PalIndex = GfxBankExtraInfo[currentGfxBank].planes;
    int i;

    for (i = 0; i < NumCols; i ++)
    {
	psel->palette[FIRST_USER_COLOR+i].r = 
	    ColPalettes[PalIndex][NumCols*current_palette_number+i].r;
	psel->palette[FIRST_USER_COLOR+i].g = 
	    ColPalettes[PalIndex][NumCols*current_palette_number+i].g;
	psel->palette[FIRST_USER_COLOR+i].b =
	    ColPalettes[PalIndex][NumCols*current_palette_number+i].b;
    }
    return D_EXIT;
}

int pal_edit_ok(DIALOG *d)
{
    int NumCols = 1 << GfxBankExtraInfo[currentGfxBank].planes;
    int PalIndex = GfxBankExtraInfo[currentGfxBank].planes;
    int i;

    for (i = 0; i < NumCols; i ++)
    {
	ColPalettes[PalIndex][NumCols*current_palette_number+i].r
		    = psel->palette[FIRST_USER_COLOR+i].r; 
	ColPalettes[PalIndex][NumCols*current_palette_number+i].g
		    = psel->palette[FIRST_USER_COLOR+i].g;
	ColPalettes[PalIndex][NumCols*current_palette_number+i].b
		    = psel->palette[FIRST_USER_COLOR+i].b;
    }

    // save palettes to the ini file
    SaveAllPalettes();

    return D_EXIT;
}

void adjust_sliders(void)
{
    palette_editor[3].d2 = psel->palette[edit_color].r;
    palette_editor[4].d2 = psel->palette[edit_color].g;
    palette_editor[5].d2 = psel->palette[edit_color].b;
}

int psel_edit_callback(DIALOG *d, int color, int mouse)
{
    d=d;
    mouse=mouse;
    edit_color = color;

    // now adjust the sliders...
    adjust_sliders();

    while(mouse_b);

    return D_REDRAW;
}

int r_slider(void *dp3, int d2)
{
    psel->palette[edit_color].r = d2;
    refresh_palette_settings();
    return D_O_K;
}

int g_slider(void *dp3, int d2)
{
    psel->palette[edit_color].g = d2;
    refresh_palette_settings();
    return D_O_K;
}

int b_slider(void *dp3, int d2)
{
    psel->palette[edit_color].b = d2;
    refresh_palette_settings();
    return D_O_K;
}

int pal_screen_text(int msg, DIALOG *d, int c)
{
    if (msg == MSG_DRAW)
    {
	// draw up all of the text onto the screen...
	text_mode(GUI_BACK);

	textprintf_centre(screen, font, d->x+58, d->y+97, GUI_FORE,
			     " Color %d ", edit_color - psel->basecolor);

	textprintf_centre(screen, font, d->x+118, d->y+97, GUI_FORE,
			     " %d ", palette_editor[3].d2);
	textprintf_centre(screen, font, d->x+143, d->y+97, GUI_FORE,
			     " %d ", palette_editor[4].d2);
	textprintf_centre(screen, font, d->x+168, d->y+97, GUI_FORE,
			     " %d ", palette_editor[5].d2);

	border_3d(screen, d->x+23, d->y+94, 68, 14, 0);
	border_3d(screen, d->x+106, d->y+94, 22, 14, 0);
	border_3d(screen, d->x+131, d->y+94, 22, 14, 0);
	border_3d(screen, d->x+156, d->y+94, 22, 14, 0);
    }

    return D_O_K;
}


int palette_edit(void)
{
    if (GameDriverLoaded == FALSE)  return D_O_K;

    // create a dialog with a color selector, and 3 sliders for red/green/blue
    edit_color = psel->basecolor;
    adjust_sliders();

    position_dialog(palette_editor, SCREEN_W-palette_editor[0].w-2,2);
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

   { pal_select_proc,  25, 25, (4*15)+4, (4*15)+4, GUI_FORE, GUI_BACK, 
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
