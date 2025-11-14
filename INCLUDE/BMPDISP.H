// bmpdisp.h
//
//  bitmap display UI module for allegro
//
//  December, 1998
//  jerry@mail.csh.rit.edu

// see the .h file for what data is to be displayed in what fields

// use a set of buttons to change the structure to move the display around

#include <../ALLEGRO.H>

// d1 gets hilight information -- hands off!

// dp gets the bitmap

// dp3 is a pointer to a callback function for mouse clicks

// dp2 gets a pointer to this structure:
typedef struct bitmap_display_struct {
    int x;     // start x,y position (what pixel to put
    int y;     //  in the upper left position
    BOOL grid; // true if gridsnap is on
    int size;  // grid spacing size 
    int sel_w; //  \___ size of the selection box
    int sel_h; //  /
    int lmx;  // \___ internals -- hands off
    int lmy;  // /
} BITMAP_DISPLAY_STRUCT;


/* custom dialog procedure for the bitmap display object */
int bitmap_display_proc(int msg, DIALOG *d, int c);
