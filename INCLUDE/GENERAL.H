// general.h
//
//  main dialog
//
//  September, 1998
//  jerry@mail.csh.rit.edu


#ifdef USE_DEGUI
#include <degui.h>
#endif

extern char title_text[];
extern char title_date[];
extern char command_line_driver[]; // if a driver was named on the command line.

// useful typedefs
typedef int BOOL;
typedef unsigned char BYTE;


// colors 1 thru 8 are used by degui
#define UI_COLOR_0 (1)
#define GUI_FORE 	(UI_COLOR_0)
#define GUI_MID 	(UI_COLOR_0 +1)
#define GUI_BACK 	(UI_COLOR_0 +2)
#define GUI_L_SHAD 	(UI_COLOR_0 +3)
#define GUI_D_SHAD 	(UI_COLOR_0 +4)
#define GUI_SELECT 	(UI_COLOR_0 +5)
#define GUI_DESELECT 	(UI_COLOR_0 +6)
#define GUI_DISABLE 	(UI_COLOR_0 +7)

// the following colors are used by the cursor function 
#define MOUSE_COLOR_DARK  (9)
#define MOUSE_COLOR_MID   (10)
#define MOUSE_COLOR_LIGHT (11)

#define COLOR_HILITE (12)
#define COLOR_LOLITE (13)

#define COLOR_RED   (14)
#define COLOR_GREEN (15)
#define COLOR_BLUE  (16)

#define FIRST_USER_COLOR (17)

// pick the color scheme for the mouse cursor
#undef  AQUA_CURSOR_COLORS    /* aqua shades */
#undef  RED_CURSOR_COLORS     /* red shades */
#define AMIGA_CURSOR_COLORS   /* amigados 2.x & 3.x  colors */

extern int gfx_hres;
extern int gfx_vres;

#define ED_DEF_SIZE (16)

#define FADE_SPEED (5)
