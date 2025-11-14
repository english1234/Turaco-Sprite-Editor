// guipal.c
//
//  Palette UI module for allegro
//
//  September, 1998
//  jerry@mail.csh.rit.edu

// see the .h file for what data is to be displayed in what fields

#include <stdio.h>
#include <memory.h>
#include "../INCLUDE/ALLEGRO.H"
#include "../INCLUDE/general.h"
#include "../INCLUDE/util.h"
#include "../INCLUDE/guipal.h"

extern MYBITMAP* screen;          // Main display bitmap
extern volatile int mouse_x;
extern volatile int mouse_y;

MYBITMAP* create_bitmap(int width, int height) {
//	printf("Creating bitmap: %dx%d\n", width, height);

	MYBITMAP* bmp = malloc(sizeof(MYBITMAP));
	if (!bmp) {
		printf("Failed to allocate MYBITMAP structure\n");
		return NULL;
	}

	// Initialize all fields to zero first
	memset(bmp, 0, sizeof(MYBITMAP));

	bmp->w = width;
	bmp->h = height;
	bmp->clip = 1;
	bmp->cl = 0;
	bmp->cr = width - 1;
	bmp->ct = 0;
	bmp->cb = height - 1;

	// Allocate pixel data
	bmp->dat = malloc(width * height);
	if (!bmp->dat) {
		printf("Failed to allocate pixel data\n");
		free(bmp);
		return NULL;
	}
	memset(bmp->dat, 0, width * height); // Initialize to zero

	// Allocate line pointers
	bmp->line = malloc(height * sizeof(unsigned char*));
	if (!bmp->line) {
		printf("Failed to allocate line pointers\n");
		free(bmp->dat);
		free(bmp);
		return NULL;
	}

	// Initialize line pointers
	for (int y = 0; y < height; y++) {
		bmp->line[y] = (unsigned char*)bmp->dat + (y * width);
	}

//	printf("Bitmap created successfully at %p\n", bmp);
	return bmp;
}

void destroy_bitmap(MYBITMAP* bmp) {
	if (bmp) {
		if (bmp->dat) free(bmp->dat);
		if (bmp->line) free(bmp->line);
		free(bmp);
	}
}

void clear_to_color(MYBITMAP* bmp, int color) {
	if (!bmp || !bmp->dat) return;

	// Fill the entire bitmap with the specified color
	unsigned char* pixels = (unsigned char*)bmp->dat;
	for (int i = 0; i < bmp->w * bmp->h; i++) {
		pixels[i] = color;
	}
}

// Fill rectangle
void rectfill(MYBITMAP* bmp, int x1, int y1, int x2, int y2, int color) {
	if (!bmp) return;

	for (int y = y1; y <= y2; y++) {
		for (int x = x1; x <= x2; x++) {
			putpixel(bmp, x, y, color);
		}
	}
}

void blit(MYBITMAP* source, MYBITMAP* dest,
	int source_x, int source_y,
	int dest_x, int dest_y,
	int width, int height)
{
	if (!source || !dest || !source->dat || !dest->dat) return;

	// Clip the blit operation
	if (source_x < 0) { width += source_x; dest_x -= source_x; source_x = 0; }
	if (source_y < 0) { height += source_y; dest_y -= source_y; source_y = 0; }
	if (dest_x < 0) { width += dest_x; source_x -= dest_x; dest_x = 0; }
	if (dest_y < 0) { height += dest_y; source_y -= dest_y; dest_y = 0; }

	if (source_x + width > source->w) width = source->w - source_x;
	if (source_y + height > source->h) height = source->h - source_y;
	if (dest_x + width > dest->w) width = dest->w - dest_x;
	if (dest_y + height > dest->h) height = dest->h - dest_y;

	if (width <= 0 || height <= 0) return;

	// Copy pixels row by row
	for (int y = 0; y < height; y++) {
		int src_row = source_y + y;
		int dst_row = dest_y + y;

		if (src_row < source->h && dst_row < dest->h) {
			unsigned char* src_ptr = (unsigned char*)source->dat + (src_row * source->w + source_x);
			unsigned char* dst_ptr = (unsigned char*)dest->dat + (dst_row * dest->w + dest_x);
			memcpy(dst_ptr, src_ptr, width);
		}
	}
}

/* custom dialog procedure for the Sprite Palette object */
int pal_display_proc(int msg, DIALOG *d, int c)
{
    MYBITMAP * temp;
    int *fg;
    int *bg;
    int ix,iy,iw,ih;

    /* process the message */

    if (msg == MSG_DRAW)
    {
    	fg = d->dp;
    	bg = d->dp2;

	/* draw in response to draw messages */
	temp = create_bitmap(d->w, d->h);
	clear_to_color(temp, *bg);

	border_3d(temp, 0, 0, temp->w, temp->h, 0);

	ix = temp->w /3;
	iy = temp->h /3;
	iw = (temp->w *2)/3;
	ih = (temp->h *2)/3;

	rectfill(temp, ix, iy, iw-1, ih-1, *fg);

	border_3d(temp, ix, iy, ix+1, iy+1, 1);

	blit(temp, screen, 0, 0, d->x, d->y, d->w, d->h);
	destroy_bitmap(temp);

    } else {
   
    	// check to see if a redraw is necessary (automatic)
    	fg = d->dp;
    	bg = d->dp2;
	if(*fg != d->d1 || *bg != d->d2)
	{
	    d->d1 = *fg;
	    d->d2 = *bg;

	    show_mouse(NULL);
	    SEND_MESSAGE(d, MSG_DRAW, 0);
	    show_mouse(screen);
	}
    }

    /* always return OK status, since we don't ever need to close 
    * the dialog or get the input focus.
    */
    return D_O_K;
}



/*  layout:
**  2 colors   4 colors   8 colors   16 colors
**  +-------+  +-------+  +---+---+  +-+-+-+-+
**  |       |  |   0   |  | 0 | 4 |  |0|4|8|C|
**  |   0   |  +-------+  +---+---+  +-+-+-+-+
**  |       |  |   1   |  | 1 | 5 |  |1|5|9|D|
**  +-------+  +-------+  +---+---+  +-+-+-+-+
**  |       |  |   2   |  | 2 | 6 |  |2|6|A|E|
**  |   1   |  +-------+  +---+---+  +-+-+-+-+
**  |       |  |   3   |  | 3 | 7 |  |3|7|B|F|
**  +-------+  +-------+  +---+---+  +-+-+-+-+
*/

// not sure how to display 32 color palettes yet... hrm.


// get index from quad (gifquad) -- pass in 16 
int pal_index_from_16(int ncolors, int quad)
{
    // if you know a better way of doing this, feel free to e-mail
    // me.  I'm sick now, and this seems like the best way to do it.
    //   -jerry
    switch (ncolors)
    {
    case(2):
	switch(quad){
	case(0): case(4): case(8):  case(12):
	case(1): case(5): case(9):  case(13):
	    return 0;
	default:
	    return 1;
	}

    case(4):
	switch(quad){
	case(0): case(4): case(8):  case(12): return(0);
	case(1): case(5): case(9):  case(13): return(1);
	case(2): case(6): case(10): case(14): return(2);
	case(3): case(7): case(11): case(15): return(3);
	}

    case(8):
	switch(quad){
	case(0): case(4): return(0);
	case(1): case(5): return(1);
	case(2): case(6): return(2);
	case(3): case(7): return(3);
	case(8):  case(12): return(4);
	case(9):  case(13): return(5);
	case(10): case(14): return(6);
	case(11): case(15): return(7);
	}

    case(16):
    	return(quad);
    }
    return 0;
}

////////////////////////////////////////
// "You punched him to death!"
// "Yeah.  I'm a freaking Jedi, man!"
////////////////////////////////////////

/* custom dialog procedure for the Palette object */
int pal_select_proc(int msg, DIALOG *d, int c)
{
    MYBITMAP * temp = NULL;
    PALETTE_SELECT * ps = NULL;
    PALETTE_SELECT ** pt = NULL;
    int qw, qh; 
    int hw, hh;
    int xx, yy;
    int curcol;
    int (*proc)(DIALOG *dl, int color, int mouse);

    /* process the message */
    switch (msg) {

    /* initialise when we get a start message */
    case MSG_START:
	d->d2 = COLOR_LOLITE;
	break;

    /* shutdown when we get an end message */
    case MSG_END:
	break;

    /* draw in response to draw messages */
    case MSG_DRAW:
	// setup the variables we need...
	pt = d->dp;
	ps = *pt;

	temp = create_bitmap(d->w, d->h);

	if (ps == NULL)
	{
	    rectfill(temp, 0, 0, temp->w, temp->h, GUI_MID);

	    box_3d(temp, temp->w, temp->h, 0,0);
	    blit(temp, screen, 0, 0, d->x, d->y, d->w, d->h);
	    destroy_bitmap(temp);
	    return D_O_K;
	}

	// compute the quarter & half widths & heights 
	qw = ( (d->w)-4 )/4;
	qh = ( (d->h)-4 )/4;
	hw = ( (d->w)-4 )/2;
	hh = ( (d->h)-4 )/2;

	// install the palette...
	set_palette_range(ps->palette, 
			ps->basecolor, ps->basecolor+ps->ncolors, 1);


	// draw the palette squares... this is gross, but it's about 
	// the only way i can think of to do this.  ick!

	// do YOU know of a better way?  e-mail me!
	//   -jerry

	rectfill(temp, 2,2,     2+qw, 2+qh, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 0));
	rectfill(temp, 2,2+qh,  2+qw, 2+hh, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 1));
	rectfill(temp, 2,2+hh,  2+qw, 2+hh+qh, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 2));
	rectfill(temp, 2,d->h-2-qh,  2+qw, d->h-2, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 3));

	rectfill(temp, 2+qw,2,     2+hw, 2+qh, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 4));
	rectfill(temp, 2+qw,2+qh,  2+hw, 2+hh, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 5));
	rectfill(temp, 2+qw,2+hh,  2+hw, 2+hh+qh, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 6));
	rectfill(temp, 2+qw,d->h-2-qh,  2+hw, d->h-2, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 7));

	rectfill(temp, 2+hw,2,     2+hw+qw, 2+qh, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 8));
	rectfill(temp, 2+hw,2+qh,  2+hw+qw, 2+hh, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 9));
	rectfill(temp, 2+hw,2+hh,  2+hw+qw, 2+hh+qh, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 10));
	rectfill(temp, 2+hw,d->h-2-qh,  2+hw+qw, d->h-2, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 11));

	rectfill(temp, 2+hw+qw,2,     d->w-2, 2+qh, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 12));
	rectfill(temp, 2+hw+qw,2+qh,  d->w-2, 2+hh, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 13));
	rectfill(temp, 2+hw+qw,2+hh,  d->w-2, 2+hh+qh, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 14));
	rectfill(temp, 2+hw+qw,d->h-2-qh,  d->w-2, d->h-2, 
	    ps->basecolor + pal_index_from_16(ps->ncolors, 15));

	// now display the 3-d effect and the highlight.
	box_3d(temp, temp->w, temp->h, 
		(d->d2 == COLOR_HILITE)?1:0,
		(d->d2 == COLOR_HILITE)?1:0 );

	blit(temp, screen, 0, 0, d->x, d->y, d->w, d->h);
	destroy_bitmap(temp);
	break; 

    case MSG_CLICK:
        // left or right mouse button was pressed.
	pt = d->dp;
	ps = *pt;

	if (ps == NULL)
	    return D_O_K;  // no palette information -- return

	xx = mouse_x - d->x - 2; // x position in the gadget
	yy = mouse_y - d->y - 2; // y position in the gadget

	if (xx < 2  || yy < 2)
	    return D_O_K;        // in the border... dump it!


	// compute the quarter & half widths & heights as above
	qw = ( (d->w)-4 )/4;
	qh = ( (d->h)-4 )/4;
	hw = ( (d->w)-4 )/2;
	hh = ( (d->h)-4 )/2;

	// figure out the horizontal quarter it's in.
	if (xx < qw)         curcol = 0x00;
	else if (xx < hw)    curcol = 0x04;
	else if (xx < hw+qw) curcol = 0x08;
	else                 curcol = 0x0C;

	// figure out the vertical quarter it's in.
	if (yy < qh)         curcol |= 0x00;
	else if (yy < hh)    curcol |= 0x01;
	else if (yy < hh+qh) curcol |= 0x02;
	else                 curcol |= 0x03;

	// convert it to a real palette value and send it to the callback

	if (d->dp2) {
	    proc = d->dp2;

	    return (*proc)(d, 
		    ps->basecolor + pal_index_from_16(ps->ncolors, curcol), 
		    c);
	}
	break; 

    case MSG_DCLICK:
	break; 

    case MSG_GOTMOUSE:
	d->d2 = COLOR_HILITE;
	show_mouse(NULL);
	SEND_MESSAGE(d, MSG_DRAW, 0);
	show_mouse(screen);
	break; 

    case MSG_LOSTMOUSE:
	d->d2 = COLOR_LOLITE;
	show_mouse(NULL);
	SEND_MESSAGE(d, MSG_DRAW, 0);
	show_mouse(screen);
	break; 

    case MSG_GOTFOCUS:
	d->d2 = COLOR_HILITE;
	break; 

    case MSG_LOSTFOCUS:
	d->d2 = COLOR_LOLITE;
	break; 

    case MSG_WANTFOCUS:
	return D_WANTFOCUS;
	break; 

    case MSG_CHAR:
	break; 
    }

    /* always return OK status, since we don't ever need to close 
    * the dialog or get the input focus.
    */
    return D_O_K;
}
