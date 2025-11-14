// GUIpal.h
//
//  Palette UI module for allegro
//
//  September, 1998
//  jerry@mail.csh.rit.edu

#include "../INCLUDE/ALLEGRO.H"

/* custom dialog procedure for the Sprite Palette object */
int pal_display_proc(int msg, DIALOG *d, int c);
// dp  is a pointer to an int containing the current foreground color
// dp2 is a pointer to an int containing the current background color

/* custom dialog procedure for the Palette selector object */
int pal_select_proc(int msg, DIALOG *d, int c);
// dp is a pointer to a palette_select structure containing the palette 
//    properly filled in. 
//
// dp2 is a pointer to a callback function with a prototype of:
//     int func(DIALOG *d, int color, int mouse);
//     which gets passed the color value [0..ncolors] of the mouse click, and
//     a flag stating whether the left or right mouse button was clicked
//     it should return D_REDRAW if the dialog is to be redrawn or D_O_K
int pal_index_from_16(int ncolors, int quad);

MYBITMAP* create_bitmap(int width, int height);
void destroy_bitmap(MYBITMAP * bmp);
void clear_to_color(MYBITMAP * bmp, int color);
void rectfill(MYBITMAP * bmp, int x1, int y1, int x2, int y2, int color);
void blit(MYBITMAP * source, MYBITMAP * dest,
	int source_x, int source_y,
	int dest_x, int dest_y,
	int width, int height);
