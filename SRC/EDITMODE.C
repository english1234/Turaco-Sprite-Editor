// editmode.c
//
//  paint mode functions
//
//  November 1998
//  jerry@mail.csh.rit.edu

#include <stdlib.h>
#include "../INCLUDE/allegro.h"
#include "../INCLUDE/general.h"
#include "../INCLUDE/editmode.h"
#include "../INCLUDE/GUIPAL.H"

// for the cursors...
MYBITMAP * cursor;
MYBITMAP * busy_cursor;
MYBITMAP * flood_cursor;
MYBITMAP * eye_cursor;
MYBITMAP * null_cursor;

// for the paint mode...
int edit_mode = MODE_PAINT;
int edit_mode_old = MODE_PAINT;

extern MYBITMAP* screen;          // Main display bitmap
extern FONT* font;              // Default system font

void set_mouse_edit_mode(int newmode)
{
    if (newmode != edit_mode)
    {
	edit_mode_old = edit_mode;
	edit_mode = newmode;

	if (edit_mode == MODE_PAINT)
	{
	    REGULAR_CURSOR();
	} else if (edit_mode == MODE_FLOODFILL) {
	    FLOOD_CURSOR();
	} else if (edit_mode == MODE_EYEDROP){
	    EYE_CURSOR();
	}
    }
}

int edit_mode_paint(void)
{
    set_mouse_edit_mode(MODE_PAINT);
    return D_O_K;
}

int edit_mode_flood(void)
{
    set_mouse_edit_mode(MODE_FLOODFILL);
    return D_O_K;
}

int edit_mode_eye(void)
{
    set_mouse_edit_mode(MODE_EYEDROP);
    return D_O_K;
}

void create_cursors(MYBITMAP * curs, MYBITMAP * busy, MYBITMAP * flood, MYBITMAP * eye)
{
    if (curs != NULL)
    {
	// this is actually the default cursor for AmigaDos 2.x
	// it's a cool cursor, so i'm stealing it. ;}
	clear(curs);

	line(curs, 0, 0, 4,  9, MOUSE_COLOR_DARK);
	line(curs, 6, 7, 9, 10, MOUSE_COLOR_DARK);

	line(curs, 0, 0, 9,  4, MOUSE_COLOR_LIGHT);
	line(curs, 7, 6, 10, 9, MOUSE_COLOR_LIGHT);

	rect(curs, 2, 2, 3, 3, MOUSE_COLOR_MID);
	rect(curs, 3, 3, 5, 5, MOUSE_COLOR_MID);
	rect(curs, 6, 4, 7, 5, MOUSE_COLOR_MID);
	rect(curs, 4, 6, 5, 7, MOUSE_COLOR_MID);
	line(curs, 8, 5, 9, 5, MOUSE_COLOR_MID);
	line(curs, 5, 8, 5, 9, MOUSE_COLOR_MID);
	line(curs, 0, 0, 9, 9, MOUSE_COLOR_MID);

	// now install the new cursor, and set the hot-spot
	set_mouse_sprite(curs);
    }

    if (busy != NULL)
    {
	// make a simple busy pointer, as i can't muster the
	// thought to come up with anything better.
	clear(busy);
	text_mode (-1);
	rectfill(busy, 0, 0, 34, 11, MOUSE_COLOR_LIGHT);
	rect(busy, 0, 0, 34, 11, 1);
	textout(busy, font, "Wait", 3,3, MOUSE_COLOR_DARK);
	textout(busy, font, "Wait", 2,2, MOUSE_COLOR_MID);
    }

    if (flood != NULL)
    {
	clear(flood);

	// the spilling paint... 
	// perhaps change MOUSE_COLOR_DARK to be the color to fill with?
	line (flood, 0,5,  0,14, MOUSE_COLOR_DARK);
	line (flood, 1,4,  1,15, MOUSE_COLOR_DARK);
	line (flood, 1,4,  3,4,  MOUSE_COLOR_DARK);

	// paint can outline
	line (flood, 3,3,  6,0,  MOUSE_COLOR_MID);
	line (flood, 7,0,  13,6, MOUSE_COLOR_MID);
	line (flood, 13,7, 9,11, MOUSE_COLOR_MID);
	line (flood, 8,11, 2,5,  MOUSE_COLOR_MID);

	// paint can body
	floodfill(flood, 8,6, MOUSE_COLOR_LIGHT);

	// paint can top
	line (flood, 3,5,  7,1,  MOUSE_COLOR_MID);

	/*
	// the paint can handle
	line (flood, 7,5,  11,1, MOUSE_COLOR_DARK);
	line (flood, 7,6,  12,1, MOUSE_COLOR_DARK);
	line (flood, 8,6,  12,2, MOUSE_COLOR_DARK);
	*/

	// the hot spot -- make it brighter
	putpixel(flood, 1, 15, MOUSE_COLOR_MID);
    }

    if (eye != NULL)
    {
	clear(eye);
	// the dropper
	line(eye, 1,0,  3,0,  MOUSE_COLOR_DARK);
	line(eye, 0,1,  0,6,  MOUSE_COLOR_DARK);
	line(eye, 4,1,  4,6,  MOUSE_COLOR_DARK);
	line(eye, 0,6,  4,6,  MOUSE_COLOR_DARK);
	floodfill(eye, 2,2,   MOUSE_COLOR_DARK);
	// the vial
	line(eye, 1,7,  1,12, MOUSE_COLOR_MID);
	line(eye, 3,7,  3,12, MOUSE_COLOR_MID);
	line(eye, 2,13, 2,14, MOUSE_COLOR_MID);
	floodfill(eye, 2,8,   MOUSE_COLOR_LIGHT);
    }

    set_mouse_sprite_focus(0,0);
}

void Init_Cursors(void)
{
    cursor       = create_bitmap(32,32);
    busy_cursor  = create_bitmap(40,32);
    eye_cursor   = create_bitmap(32,32);
    flood_cursor = create_bitmap(32,32);
    null_cursor  = create_bitmap(2,2);

    create_cursors(cursor, busy_cursor, flood_cursor, eye_cursor);
    clear(null_cursor);
}

void DeInit_Cursors(void)
{
    destroy_bitmap(cursor);
    destroy_bitmap(busy_cursor);
    destroy_bitmap(flood_cursor);
    destroy_bitmap(eye_cursor);
    destroy_bitmap(null_cursor);
}

void busy(void)
{
    BUSY_CURSOR();
}

void empty_cursor(void)
{
    SET_CURSOR(null_cursor, 0, 0);
}

void not_busy(void)
{
    if (edit_mode == MODE_PAINT)
    {
	REGULAR_CURSOR();
    } else if (edit_mode == MODE_FLOODFILL) {
	FLOOD_CURSOR();
    } else if (edit_mode == MODE_EYEDROP){
	EYE_CURSOR();
    }
}
