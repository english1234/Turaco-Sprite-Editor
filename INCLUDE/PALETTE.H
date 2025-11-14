// palette.h
//
//  palette tools
//
//  September, 1998
//  jerry@mail.csh.rit.edu

#include "../INCLUDE/allegro.h"

// do not use these.  To change or read fg & bg colors, use the functions.
// don't read from these directly
extern int pal_fg_color;
extern int pal_bg_color;

extern int current_palette_number;
/// max: NumColPalettes[GfxBankExtraInfo[currentGfxBank].planes]

// palette edit functions...
int palette_add_new(void);
int palette_edit(void);

// the current palette (for the dialog function)
extern PALETTE_SELECT * psel;

// the palette callback(for the dialog function)
int psel_callback(DIALOG *d, int color, int mouse);

// get/set the colors
void Set_BG_Color(int color);
void Set_FG_Color(int color);

int Get_BG_Color(void);
int Get_FG_Color(void);

// bookkeeping
void Init_Palette(void);
void DeInit_Palette(void);

// gui util functions...
int pal_plus(DIALOG *d);
int pal_minus(DIALOG *d);

int palette_inc(void);
int palette_dec(void);
void palette_reset(void);
