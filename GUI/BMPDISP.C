// bmpdisp.c
//
//  bitmap display UI module for allegro
//
//  December, 1998
//  jerry@mail.csh.rit.edu

// see the .h file for what data is to be displayed in what fields

// use a set of buttons to change the structure to move the display around

#include <stdlib.h>
#include "../INCLUDE/ALLEGRO.H"
#include "../INCLUDE/general.h"
#include "../INCLUDE/bmpdisp.h"
#include "../INCLUDE/util.h"
#include "../INCLUDE/editmode.h"
#include "../INCLUDE/GUIPAL.H"

#if 0
// dp gets the bitmap
// dp2 gets this structure
// dp3 is a pointer to a callback function for mouse clicks
struct bitmap_display_struct {
    int x;     // start x,y position (what pixel to put
    int y;     //  in the upper left position
    BOOL grid; // true if gridsnap is on
    int size;  // grid spacing size 
    int sel_w; //  \___ size of the selection box
    int sel_h; //  /
    int lmx;  // \___ internals -- hands off
    int lmy;  // /
};
#endif

extern MYBITMAP* screen;          // Main display bitmap
extern FONT* font;              // Default system font
extern volatile int mouse_x;
extern volatile int mouse_y;

extern MYBITMAP* create_bitmap(int width, int height);
extern void show_mouse(MYBITMAP* bmp);
extern void text_mode(int mode);

/* custom dialog procedure for the bitmap display object */
int bitmap_display_proc(int msg, DIALOG *d, int c)
{
    MYBITMAP * blitme;
    MYBITMAP ** temp1;
    MYBITMAP * temp2;
    BITMAP_DISPLAY_STRUCT *bds  = NULL;
    BITMAP_DISPLAY_STRUCT **bdt = NULL;
    int x,y;
    int icx, icy;
    int ret = D_O_K;
    int (*proc)(DIALOG *dl, int x_pos, int y_pos, int mouse);

    /* process the message */
    switch (msg) {

    /* initialise when we get a start message */
    case MSG_START:
	if (d->dp2)
	{
	    bdt = d->dp2;
	    bds = *bdt;
	    bds->lmx = bds->lmy = -1;
	    bds->size = 8;
	}
	d->d1 = COLOR_LOLITE;
	break;

    /* shutdown when we get an end message */
    case MSG_END:
	not_busy();
	break;

    /* draw in response to draw messages */
    case MSG_DRAW:
	blitme = create_bitmap(d->w, d->h);
	clear_to_color(blitme, GUI_BACK);

	if (d->dp == NULL)
	{
	    text_mode (-1);
	    textout_centre(blitme, font, "NO BMP!", 
	                   (d->w)/2, (d->h)/2, GUI_FORE);
	} else {
	    temp1 = d->dp;  // pointer to the bitmap to draw
	    temp2 = *temp1; // this is the bitmap to draw
	    if (temp2 == NULL)
	    {
		text_mode (-1);
		textout_centre(blitme, font, "NO BMP!", 
		               blitme->w/2, blitme->h/2, GUI_FORE);
	    } else {
		// draw the bitmap here...
		if (d->dp2 == NULL)
		{
		    // blit (d, s, sx, sy, dx, dy, w, h);
		    blit(temp2, blitme, 0, 0, 
			    2, 2, blitme->w-4, blitme->h-4);
		} else {
		    bdt = d->dp2;
		    bds = *bdt;

		    blit(temp2, blitme, bds->x, bds->y,
			    2, 2, blitme->w-4, blitme->h-4);

		    // now draw the cursor position
		    if (mouse_x < d->x+temp2->w-bds->x && 
			mouse_y < d->y+temp2->h-bds->y )
		    {
			icx = mouse_x-d->x;
			icy = mouse_y-d->y;
		    } else {
			icx = -1;
			icy = -1;
		    }

		    if (icx != -1)
		    {
			my_draw_dotted_rect(blitme, 
			     icx-1, icy-1, 
			     icx + bds->sel_w, icy + bds->sel_h, 
			     COLOR_HILITE);
		    }

		    // ... or dont.
		}
	    }
	}

	box_3d(blitme, blitme->w, blitme->h, 
		(d->d1 == COLOR_HILITE)?1:0,
		(d->d1 == COLOR_HILITE)?1:0
		);

	blit(blitme, screen, 0, 0, d->x, d->y, d->w, d->h);
	destroy_bitmap(blitme);
	break; 

    case MSG_CLICK:
	/* call callback function here */
	x = mouse_x - d->x - 2;
	y = mouse_y - d->y - 2;

	if (d->dp2)
	{
	    bdt = d->dp2;
	    bds = *bdt;

	    x += bds->x; 
	    y += bds->y; 
	}

	if (d->dp3) {
	    proc = d->dp3;
	    return (*proc)(d, x, y, c);
	}

	break; 

    case MSG_DCLICK:
	break; 

    case MSG_GOTMOUSE:
	d->d1 = COLOR_HILITE;
	if (d->dp2)
	{
	    bdt = d->dp2;
	    bds = *bdt;
	    bds->lmx = mouse_x;
	    bds->lmy = mouse_y;
	}
	not_busy();
	show_mouse(NULL);
	SEND_MESSAGE(d, MSG_DRAW, 0);
	show_mouse(screen);
	break; 

    case MSG_LOSTFOCUS:
    case MSG_LOSTMOUSE:
	d->d1 = COLOR_LOLITE;
	if (d->dp2)
	{
	    bdt = d->dp2;
	    bds = *bdt;
	    bds->lmx = bds->lmy = -1;
	}
        not_busy();
	show_mouse(NULL);
	SEND_MESSAGE(d, MSG_DRAW, 0);
	show_mouse(screen);
	break; 

    case MSG_GOTFOCUS:
	d->d1 = COLOR_HILITE;
	not_busy();
	break; 

    case MSG_WANTFOCUS:
	return D_WANTFOCUS;
	break; 

    case MSG_IDLE:
        if (d->d1 == COLOR_HILITE || 
            (
              (mouse_x >= d->x && mouse_x <= d->x + d->w) &&
              (mouse_y >= d->y && mouse_y <= d->y + d->h) 
            ) 
           )

        {
	    if (d->dp && d->dp2)
	    {
		temp1 = d->dp;  // pointer to the bitmap to draw
		temp2 = *temp1; // this is the bitmap to draw

		bdt = d->dp2;
		bds = *bdt;
		c = 0; // we can use this as a temp variable..

		if (mouse_x < d->x+temp2->w-bds->x && 
		    mouse_y < d->y+temp2->h-bds->y )
		{
		    if (mouse_x != bds->lmx  || mouse_y != bds->lmy)
		    {
			empty_cursor();
			bds->lmx = mouse_x;
			bds->lmy = mouse_y;
			c = 1;
		    }
		} else {
		    if (bds->lmx != -1)
		    {
			not_busy();
			bds->lmx = -1;
			bds->lmy = -1;
			c = 1;
		    }
		}

		if (c)
		{
		    show_mouse(NULL);
		    SEND_MESSAGE(d, MSG_DRAW, 0);
		    show_mouse(screen);
		}
	    }
	} else {
	    if (d->dp2)
	    {
		bdt = d->dp2;
		bds = *bdt;
		if (bds->lmx != -1)
		{
		    not_busy();
		    bds->lmx = bds->lmy = -1;
		    show_mouse(NULL);
		    SEND_MESSAGE(d, MSG_DRAW, 0);
		    show_mouse(screen);
		}
	    }
	}
	break; 

    case MSG_CHAR:
	if (d->dp2 && d->dp)
	{
	    bdt = d->dp2;
	    bds = *bdt;

	    temp1 = d->dp;  // pointer to the bitmap to draw
	    temp2 = *temp1; // this is the bitmap to draw

	    switch(c >> 8){
	    case(KEY_UP):
		if (bds->y <= temp2->h-bds->size)
		{
		    bds->y += bds->size;
		}
		c = 0;
		break;
	    case(KEY_DOWN):
		if (bds->y) 
		{
		    bds->y -= bds->size;
		    if (bds->y <0)
			bds->y = 0;
		}
		c = 0;
		break;
	    case(KEY_LEFT):
		if (bds->x <= temp2->w-bds->size)
		{
		    bds->x += bds->size;
		}
		c = 0;
		break;
	    case(KEY_RIGHT):
		if (bds->x) 
		{
		    bds->x -= bds->size;
		    if (bds->x <0)
			bds->x = 0;
		}
		c = 0;
		break;
	    }

	    if (!c)  // if we used the character...
	    {
		show_mouse(NULL);
		SEND_MESSAGE(d, MSG_DRAW, 0);
		show_mouse(screen);
		return (D_USED_CHAR);  // eat it.
	    }
	}
	break; 
    }

    /* always return OK status, since we don't ever need to close 
    * the dialog or get the input focus.
    */
    return ret;
}
