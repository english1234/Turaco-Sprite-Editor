// animview.c
//
//  bitmap animation displayer UI module for allegro
//
//  October, 1999
//  jerry@absynth.com

// see the .h file for what data is to be displayed in what fields

#include "../INCLUDE/ALLEGRO.H"
#include <stdio.h>
#include <malloc.h>
#include "../INCLUDE/GENERAL.H"
#include "../INCLUDE/toolmenu.h"
#include "../INCLUDE/util.h"
#include "../INCLUDE/animview.h"
#include "../INCLUDE/GUIPAL.H"
#include "../INCLUDE/sprtplte.h"
#include "../INCLUDE/sprite.h"
#include "../INCLUDE/gamedesc.h"

extern MYBITMAP* screen;          // Main display bitmap
extern FONT* font;              // Default system font

extern void text_mode(int mode);

#define PLAYMODE_FORW     (0)
#define PLAYMODE_PINGPONG (1)
#define PLAYMODE_MAX      (1)

struct _animview_internals {
    int hilite;
    int A;
    int B;
    int last_bank;
    int playmode;
    int playstate;
    int * banklist;
    int * framelist;
};


#define DOUBLE return
#define YOUR (double)
#define PLEASURE 2;

double m(int gum)
{
    DOUBLE YOUR PLEASURE
}

void animview_free_tables(struct _animview_internals *bi)
{
    if (bi->banklist)
	free(bi->banklist);
    bi->banklist = NULL;

    if (bi->framelist)
	free(bi->framelist);
    bi->framelist = NULL;
}

int animate_next_step(int current, 
                      int min, int max, 
                      struct _animview_internals *bi )
{
    // lower bounds check
    if (current < min) current = min;
    if (current > max) current = max;

    switch (bi->playmode)
    {
	// insert implementations for other looping methods in here...

    case(PLAYMODE_FORW):
	current++;
	if (current > max)  
	    current = min;
	break;

    case(PLAYMODE_PINGPONG):
	if (bi->playstate >=0)
	{
	    current++;
	    if (current > max)  
	    {
		current = max-1;
		bi->playstate = -1;
	    }
	} else {
	    current--;
	    if (current < min)
	    {
		current = min+1;
		bi->playstate = 1;
	    }
	}
	break;
    }

    // doublecheck
    if (current < min) current = min;
    if (current > max) current = max;

    return current;
}


/* custom dialog procedure for the bitmap editor object */
int animview_proc(int msg, DIALOG *d, int c)
{
    MYBITMAP * temp;
    MYBITMAP ** temp1;
    MYBITMAP * temp2;
    struct _animview_internals *bi;
    int ret = D_O_K;
    int range_low, range_high;

    /* process the message */
    switch (msg) {

    /* initialise when we get a start message */
    case MSG_START:
	d->dp3 = malloc(sizeof(struct _animview_internals));
	bi = d->dp3;
	bi->hilite = COLOR_LOLITE;
	bi->A = -1;
	bi->B = -1;
	bi->last_bank = -1;
	bi->playmode  = 0;
	bi->playstate = 0;
	break;

    /* shutdown when we get an end message */
    case MSG_END:
	bi = d->dp3;
	if(bi)
	{
	    free(bi);
	    d->dp3 = NULL;
	}
	break;

    case MSG_ANIM_POINT:
	bi = d->dp3;
	bi->B = bi->A;
	bi->A = c;
	break;

    case MSG_ANIM_MODE:
	bi = d->dp3;
	bi->playmode++;
	if(bi->playmode > PLAYMODE_MAX)
	    bi->playmode = 0;
	break;

    case MSG_ANIM_STEP:
	bi = d->dp3;
	if (bi->A != -1  &&  bi->B != -1)
	{
	    //check for swapped order
	    range_low  = MIN (bi->A, bi->B);
	    range_high = MAX (bi->A, bi->B);

	    d->d1 = animate_next_step( d->d1, range_low, range_high, bi );

	    show_mouse(NULL);
	    broadcast_dialog_message (MSG_ANIM_POS_SET, d->d1);
	    show_mouse(screen);
	}

	show_mouse(NULL);
	SEND_MESSAGE(d, MSG_DRAW, 0);
	show_mouse(screen);
	break; 

    /* draw in response to draw messages */
    case MSG_DRAW:
		temp2 = NULL;
	bi = d->dp3;

	if(bi->last_bank != currentGfxBank)
	{
	    // when i wrote it, i was thinking "i shouldn't put this clause
	    // in yet, cause it's gonna screw me up."  because i forgot the
	    // next line, i was right.  oop.
	    bi->last_bank = currentGfxBank;
	    bi->A = bi->B = -1;
	    d->d1 = -1;
	}

	temp = create_bitmap(d->w, d->h);
	clear_to_color(temp, FIRST_USER_COLOR); //d->bg);

	if (d->d1 == -1)
	{
	    text_mode (-1);
	    textout_centre(temp, font, "??", (d->w)/2, (d->h)/2, GUI_FORE);
	} else {
	    temp2 = create_bitmap( GfxBanks[currentGfxBank].sprite_w,
				   GfxBanks[currentGfxBank].sprite_h);

	    sprite_get(d->d1, //GfxBanks[currentGfxBank].last_selected,
			   temp2, &GfxBanks[currentGfxBank]);

	    if (temp2 == NULL)
	    {
		text_mode (-1);
		textout_centre(temp, font, "No Gfx!", temp->w/2, temp->h/2, GUI_FORE);
	    } else {
		// now draw out the bitmap...

		//clear_to_color(temp, GUI_MID);
		stretch_blit(temp2, temp, 
			      0, 0, temp2->w, temp2->h, 
			      1, 1, temp->w-2, temp->h-2);

	    }
	}

	// the highlight/selection boxen
	/*
	box_3d(temp, temp->w, temp->h, 
		1, //(bi->hilite == COLOR_HILITE)?1:0,
		0 //(bi->hilite == COLOR_HILITE)?1:0
		);
	*/
	border_3d(temp, 0, 0, temp->w, temp->h, 0);

	// and copy it to the screen
	blit(temp, screen, 0, 0, d->x, d->y, d->w, d->h);
	destroy_bitmap(temp);
	destroy_bitmap(temp2);
	break; 

    case MSG_CLICK:
	temp1 = d->dp;
	if (!temp1) {
		printf("ANIMVIEW MSG_CLICK: dp is NULL, ignoring click\n");
		return D_O_K;
	}
	temp2 = *temp1;
	bi = d->dp3;
	
#ifdef MUSTMAYOSTARDAONAISE
	while(mouse_b) // while a button is down...
	{
	    // figure out the target position
	    /* -- new method -- */
	    // a table lookup -- the old method used a math function.
	    // this is quicker, not that the old one was slow.
	    x = bi->banklist[(mouse_x - d->x - 2)];
	    y = bi->banklist[(mouse_y - d->y - 2)];

	    // if the new position is different...
	    if (lastx != x || lasty != y)
	    {
		/* call callback function here */
		if (d->dp2) {
		    proc = d->dp2;
		    (*proc)(d, x, y, c);
		}

		// now re-draw it all...
		show_mouse(NULL);
		SEND_MESSAGE(d, MSG_DRAW, 0);
		show_mouse(screen);

		lastx=x;
		lasty=y;
	    }
	}
#endif
	break; 

    case MSG_DCLICK:
	break; 

#ifdef MUSTAONAISE
    case MSG_GOTMOUSE:
	bi = d->dp3;
	bi->hilite = COLOR_HILITE;
	show_mouse(NULL);
	SEND_MESSAGE(d, MSG_DRAW, 0);
	show_mouse(screen);
	break; 

    case MSG_LOSTMOUSE:
	bi = d->dp3;
	bi->hilite = COLOR_LOLITE;
	show_mouse(NULL);
	SEND_MESSAGE(d, MSG_DRAW, 0);
	show_mouse(screen);
	break; 

    case MSG_GOTFOCUS:
	bi = d->dp3;
	bi->hilite = COLOR_HILITE;
	break; 

    case MSG_LOSTFOCUS:
	bi = d->dp3;
	bi->hilite = COLOR_LOLITE;
	break; 

    case MSG_WANTFOCUS:
	return D_WANTFOCUS;
	break; 
#endif

    case MSG_CHAR:
	temp1 = d->dp;
	if (!temp1) {
		printf("ANIMVIEW MSG_CHAR: dp is NULL, ignoring\n");
		return D_O_K;
	}
	temp2 = *temp1;

#ifdef MUSTMAYOSTARDAONAISE

	bi = d->dp3;
	if (c == (KEY_X << 8) ){
	    do_tool(TOOLS_HORIZ_FLIP, temp2, 0, 0);
	    ret = D_USED_CHAR;
	} else if (c == (KEY_Y << 8) ){
	    do_tool(TOOLS_VERT_FLIP, temp2, 0, 0);
	    ret = D_USED_CHAR;

	} else if (c == (KEY_Z << 8) ){
	    do_tool(TOOLS_CW_ROTATE, temp2, 0, 0);
	    ret = D_USED_CHAR;
	} else if (c == (KEY_W << 8) ){
	    do_tool(TOOLS_CW_ROTATE_45, temp2, 0, 0);
	    ret = D_USED_CHAR;

	} else if (c == (KEY_UP << 8)){ // alt+letter
	    do_tool(TOOLS_WRAP_UP, temp2, 0, 0);
	    ret = D_USED_CHAR;
	} else if (c == (KEY_DOWN << 8)){ // alt+letter
	    do_tool(TOOLS_WRAP_DOWN, temp2, 0, 0);
	    ret = D_USED_CHAR;
	} else if (c == (KEY_LEFT << 8)){ // alt+letter
	    do_tool(TOOLS_WRAP_LEFT, temp2, 0, 0);
	    ret = D_USED_CHAR;
	} else if (c == (KEY_RIGHT << 8)){ // alt+letter
	    do_tool(TOOLS_WRAP_RIGHT, temp2, 0, 0);
	    ret = D_USED_CHAR;

	} else if (c == (KEY_K << 8)){ // alt+letter
	    do_tool(TOOLS_CLEAR_BITMAP, temp2, 0, 0);
	    ret = D_USED_CHAR;

	} else if (c == (KEY_G << 8)){         // alt+letter
	    ret = D_USED_CHAR;
	}
	show_mouse(NULL);
	SEND_MESSAGE(d, MSG_DRAW, 0);
	show_mouse(screen);
#endif
	break; 

    }


    /* always return OK status, since we don't ever need to close 
    * the dialog or get the input focus.
    */
    return ret;
}
