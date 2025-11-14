// sprtplte.h
//
//  Sprite Palette UI module for allegro
//
//  September, 1998
//  jerry@mail.csh.rit.edu

#include "../INCLUDE/ALLEGRO.H"

#define SPRITE_FLAG_NEW   (0x01)
// if there is a change in the bitmap or bank, the spm (d->dp) structure (below)
// that the gui object knows about should have this bit set in the flags

typedef struct sprite_palette{
    MYBITMAP * bmp;  // the bitmap containing the graphics bank
    int sprite_w;  // width of a sprite
    int sprite_h;  // height of a sprite
    int n_total;   // number of sprites in the bitmap

    // for navigation:
    int first_sprite; // the first sprite on the top row(set on bank switch)
    int n_per_row;    // number of sprites per row (computed)
    int n_rows;       // the number of rows (mirror of internal's) (computed)

    int last_selected;  // last selected character
    int mouse_over;     // the sprite that the mouse is over
    int anim_pos;       // the animation position

    // flags..
    int flags;     // control and status flags
} SPRITE_PALETTE;
// this assumes that the bitmap has every sprite across its width
// and is as tall as sprite_h

#define MSG_ANIM_POS_SET (MSG_USER+42)

void sprite_get(int which_one, MYBITMAP * a_sprite, SPRITE_PALETTE * sp);
// gets (which_one) sprite from the palette in sp, and puts it into
// a_sprite, which MUST be large enough for the sprite -- otherwise it 
// will get clipped
// a_sprite contains the bitmap from (0,0) to  (sprite_w, sprite_h)


void sprite_put(int which_one, MYBITMAP * a_sprite, SPRITE_PALETTE * sp);
// puts (which_one) sprite into the palette in sp, and gets it from
// a_sprite, which MUST be large enough for the sprite -- otherwise it 
// will get clipped 
// a_sprite contains the bitmap from (0,0) to  (sprite_w, sprite_h)


/* custom dialog procedure for the Sprite Palette object */
int sprite_palette_proc(int msg, DIALOG *d, int c);
//  dp  points to a pointer to a sprite_palette structure (above)
//  dp2 points to a function for when the mouse is clicked.
//  dp3 points to a sprite_palette_internals structure (no touchy!)
//
//  if the dp structure is changed in any way, set the SPRITE_FLAG_NEW flag.

