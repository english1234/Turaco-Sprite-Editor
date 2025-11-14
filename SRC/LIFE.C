// life.c
//
//  Conway's Game Of Life
//  
//   Why is this here?
//   Because it's here.  Roll the bones.
//
//  March, 1999
//  jerry@absynth.com

#include <stdio.h>

#include "../INCLUDE/ALLEGRO.H"
#include "../INCLUDE/general.h"
#include "../INCLUDE/guipal.h"
#include "../INCLUDE/palette.h"    // include this after guipal.h!!!

long life_counter = 0l;

extern MYBITMAP* current_sprite;

extern MYBITMAP* create_bitmap(int width, int height);

int do_life(void)
{
    int x,y,w,h;
    int c=0;
    int old;
    int numcolors;
    MYBITMAP * temp;
    if (current_sprite == NULL) return D_O_K;

    temp = create_bitmap(current_sprite->w, current_sprite->h);
    if (temp == NULL) return D_O_K;

    numcolors = psel->ncolors;

    w = temp->w;
    h = temp->h;
    for (x=0; x<w ; x++)
	for (y=0; y<h ; y++)
	{
	    c  = (getpixel(current_sprite, (w+x-1)%w, (h+y-1)%h) > FIRST_USER_COLOR)?1:0;
	    c += (getpixel(current_sprite,    x     , (h+y-1)%h) > FIRST_USER_COLOR)?1:0;
	    c += (getpixel(current_sprite, (w+x+1)%w, (h+y-1)%h) > FIRST_USER_COLOR)?1:0;

	    c += (getpixel(current_sprite, (w+x-1)%w,    y     ) > FIRST_USER_COLOR)?1:0;
	    c += (getpixel(current_sprite, (w+x+1)%w,    y     ) > FIRST_USER_COLOR)?1:0;

	    c += (getpixel(current_sprite, (w+x-1)%w, (h+y+1)%h) > FIRST_USER_COLOR)?1:0;
	    c += (getpixel(current_sprite,    x     , (h+y+1)%h) > FIRST_USER_COLOR)?1:0;
	    c += (getpixel(current_sprite, (w+x+1)%w, (h+y+1)%h) > FIRST_USER_COLOR)?1:0;

	    old = getpixel(current_sprite, x, y);

	    if (c >= 4) old=FIRST_USER_COLOR;
	    if (c == 3) old=MIN( (old + 1),  (FIRST_USER_COLOR+numcolors-1));
	    if (c <  2) old=FIRST_USER_COLOR;

	    putpixel(temp, x, y, old);
	}

    blit(temp, current_sprite, 0, 0, 0, 0, temp->w, temp->h);
    destroy_bitmap(temp);

    life_counter++;

    return D_REDRAW;
}
