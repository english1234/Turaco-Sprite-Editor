// toolmenu.c
//
//  bitmap manipulations tools
//
//  November 1997  &  September 1998
//  jerry@mail.csh.rit.edu

#include <stdio.h> 
#include <stdbool.h>
#include "../INCLUDE/ALLEGRO.H"
#include "../INCLUDE/general.h"
#include "../INCLUDE/guipal.h"
#include "../INCLUDE/palette.h"
#include "../INCLUDE/toolmenu.h"
#include "../INCLUDE/life.h"
#include "../INCLUDE/animate.h"

// these are for the wrap/slide function, and are not necessarily public
#define CHOOSE_SLIDE 1
#define CHOOSE_WRAP  2


// these will eventually be filled in by the calling function
int x = 2;
int y = 2;

extern MYBITMAP* current_sprite;
extern MYBITMAP* create_bitmap(int width, int height);
extern PALETTE desktop_palette;
extern Uint32 palette_rgb[256];

void tools_clear_to_background(MYBITMAP * CurChar)
{
   // Clear bitmap and current drawing
    clear_to_color(CurChar, Get_BG_Color());
}

void tools_paint(MYBITMAP* CurChar, int x, int y, int fgbg)
{
    printf("=== TOOLS_PAINT ===\n");
    printf("Sprite: %p, Coords: (%d,%d), Mode: %s\n",
        CurChar, x, y,
        (fgbg == TOOLS_PAINT_FG) ? "FG" : "BG");

    if (!CurChar) {
        printf("ERROR: CurChar is NULL!\n");
        return;
    }

    printf("Sprite dimensions: %dx%d\n", CurChar->w, CurChar->h);

    // Get current pixel before change
    int old_color = getpixel(CurChar, x, y);
    printf("Current pixel color: %d\n", old_color);

    // Get target color
    int color = (fgbg == TOOLS_PAINT_FG) ? Get_FG_Color() : Get_BG_Color();
    printf("Target color: %d\n", color);

    // Set the pixel
    printf("Calling putpixel(%p, %d, %d, %d)\n", CurChar, x, y, color);
    int result = putpixel(CurChar, x, y, color);
    printf("putpixel returned: %d\n", result);

    // Immediate verification
    int new_color = getpixel(CurChar, x, y);
    printf("New pixel color: %d -> %s\n",
        new_color,
        (new_color == color) ? "SUCCESS" : "FAILED");

    if (new_color != color) {
        printf("CRITICAL: Pixel not set correctly!\n");
    }

    printf("=== END TOOLS_PAINT ===\n");

}

void tools_flood_fill(MYBITMAP * CurChar, int x, int y, int fgbg)
{
    if (fgbg == TOOLS_FLOOD_FILL_FG)
	floodfill(CurChar, x, y, Get_FG_Color());
    else if (fgbg == TOOLS_FLOOD_FILL_BG)
	floodfill(CurChar, x, y, Get_BG_Color());
}

void tools_eyedropper(MYBITMAP * CurChar, int x, int y, int fgbg)
{
    if (fgbg == TOOLS_EYEDROPPER_FG)
	Set_FG_Color( getpixel(CurChar, x,y) );
    else if (fgbg == TOOLS_EYEDROPPER_BG)
	Set_BG_Color( getpixel(CurChar, x,y) );
}

void tools_flip(MYBITMAP * CurChar, int direction)
{
    MYBITMAP * tbmp = create_bitmap(CurChar->w,CurChar->h);

    // do the necessary manipulation on CurChar

    // let allegro flip it for us!  whee!
    blit(CurChar, tbmp, 0,0, 0,0, CurChar->w,CurChar->h);
    clear_to_color(CurChar, 0 );

    // we need to clear these bitmaps in here, cause the sprite routine
    // is basically a blit with transparency... good for games, bad for us.
    if (direction == TOOLS_VERT_FLIP)
	draw_sprite_v_flip(CurChar, tbmp, 0, 0);
    else if (direction == TOOLS_HORIZ_FLIP)
	draw_sprite_h_flip(CurChar, tbmp, 0, 0);
    else
	draw_sprite_vh_flip(CurChar, tbmp, 0, 0);
    destroy_bitmap(tbmp);
}

void tools_cw_rotate(MYBITMAP * CurChar)
{
    int sx,sy;
    MYBITMAP * tbmp = create_bitmap(CurChar->w,CurChar->h);

    // NOTE: I couldn't use allegro's rotate sprite function.  Allegro's
    //       function worked fine for rotating the bitmap, but it was
    //       inaccurate for rotating such small bitmaps with precsision.
    //       If you were to do a rotate 90 four times, you would lose
    //       the upper and leftmost row of pixels.  Yow!  (And yes, I was
    //       using the function properly)  This is a couple lines of
    //       code more than that, but it works flawlessly on any size
    //       bitmap/sprite.  -jerry

    blit(CurChar, tbmp, 0,0, 0,0, CurChar->w,CurChar->h);
    for(sx=0 ; sx<CurChar->w; sx++)
	for(sy=0 ; sy<CurChar->h; sy++)
	    putpixel(CurChar, CurChar->w-sx-1, sy, getpixel(tbmp, sy,sx));

    destroy_bitmap(tbmp);
}

void tools_rotate_angle(MYBITMAP * CurChar, int degrees)
{
    MYBITMAP * tbmp = create_bitmap(CurChar->w,CurChar->h);

    // do the necessary manipulation on CurChar

    // let allegro rotate it for us!  whee!
    blit(CurChar, tbmp, 0,0, 0,0, CurChar->w,CurChar->h);

    // we need to clear these bitmaps in here, cause the sprite routine
    // is basically a blit with transparency... good for games, bad for us.
    clear_to_color(CurChar, Get_BG_Color());

    rotate_sprite(CurChar, tbmp, 0, 0, itofix(degrees));

    destroy_bitmap(tbmp);
}

void tools_slide(MYBITMAP * CurChar, int osx, int osy, int odx, int ody, int fmt)
{
    MYBITMAP * tbmp = create_bitmap(CurChar->w,CurChar->h);

    // let allegro do it for us!  whee!
    blit(CurChar, tbmp, 0,0, 0,0, CurChar->w ,CurChar->h);
    clear_to_color(CurChar, Get_BG_Color());
    blit(tbmp, CurChar, osx,osy, odx,ody, CurChar->w,CurChar->h);

    if (fmt == CHOOSE_WRAP) {
	if (osx) blit(tbmp, CurChar, 0,0,  
			CurChar->w- osx, 0, CurChar->w, CurChar->h);
	if (odx) blit(tbmp, CurChar, 0,0, 
			-CurChar->w+ odx, 0, CurChar->w, CurChar->h);
	if (osy) blit(tbmp, CurChar, 0,0, 
			0, CurChar->h- osy, CurChar->w, CurChar->h);
	if (ody) blit(tbmp, CurChar, 0,0, 
			0,-CurChar->h+ ody, CurChar->w, CurChar->h);
    }

    destroy_bitmap(tbmp);
}



//////////////////////////////////////



void do_tool(int selection, MYBITMAP * CurrentChar, int x, int y)
{
    if (CurrentChar == NULL) {
        printf("do_tool: CurrentChar not allocated\n");
        return;
    }

    switch(selection) {
    case(TOOLS_VERT_FLIP): // vertical Flip
	tools_flip(CurrentChar, TOOLS_VERT_FLIP);
	break;
    
    case(TOOLS_HORIZ_FLIP): // horizontal Flip
	tools_flip(CurrentChar, TOOLS_HORIZ_FLIP);
	break;
    
    case(TOOLS_CW_ROTATE): // cw rotate
	tools_cw_rotate(CurrentChar);
	break;
	
    case(TOOLS_CW_ROTATE_45): // cw rotate 45
	tools_rotate_angle(CurrentChar, 32);
	break;

    case(TOOLS_CCW_ROTATE_45): // ccw rotate 45
	tools_rotate_angle(CurrentChar, -32);
	break;
	
    case(TOOLS_SLIDE_UP): // slide up
	tools_slide(CurrentChar, 0,1,0,0, CHOOSE_SLIDE);
	break;
	
    case(TOOLS_SLIDE_DOWN): // slide down
	tools_slide(CurrentChar, 0,0,0,1, CHOOSE_SLIDE);
	break;
	
    case(TOOLS_SLIDE_LEFT): // slide left
	tools_slide(CurrentChar, 1,0,0,0, CHOOSE_SLIDE);
	break;
    
    case(TOOLS_SLIDE_RIGHT): // slide right
	tools_slide(CurrentChar, 0,0,1,0, CHOOSE_SLIDE);
	break;
    
    case(TOOLS_WRAP_UP): // wrap up
	tools_slide(CurrentChar, 0,1,0,0, CHOOSE_WRAP);
	break;
	
    case(TOOLS_WRAP_DOWN): // wrap down
	tools_slide(CurrentChar, 0,0,0,1, CHOOSE_WRAP);
	break;
	
    case(TOOLS_WRAP_LEFT): // wrap left
	tools_slide(CurrentChar, 1,0,0,0, CHOOSE_WRAP);
	break;

    case(TOOLS_WRAP_RIGHT): // wrap right
	tools_slide(CurrentChar, 0,0,1,0, CHOOSE_WRAP);
	break;

    case(TOOLS_FLOOD_FILL_FG): // flood fill
    case(TOOLS_FLOOD_FILL_BG): // flood fill
	tools_flood_fill(CurrentChar, x, y, selection);
	break;

    case (TOOLS_CLEAR_BITMAP): // clear bitmap
	tools_clear_to_background(CurrentChar);
	break;

    case (TOOLS_EYEDROPPER_FG): // eyedropper (get color)
    case (TOOLS_EYEDROPPER_BG): // eyedropper (get color)
	tools_eyedropper(CurrentChar, x, y, selection);
	break;
       
    case (TOOLS_PAINT_FG): // paint foreground color
    case (TOOLS_PAINT_BG): // paint background color
	tools_paint(CurrentChar, x, y, selection);
	break;
       
    case (TOOLS_LIFE): // Conway's Game Of Life
	do_life();
	break;
       
    default:
	break;
    }

//    redraw_all(main_dialog);
}


/////////////// The menu callback functions...


int tools_clear_bg(void)
{
    do_tool(TOOLS_CLEAR_BITMAP, current_sprite, 0, 0);
    return D_REDRAW;
}



int tools_flip_h(void)
{
    do_tool(TOOLS_HORIZ_FLIP, current_sprite, 0, 0);
    return D_REDRAW;
}

int tools_flip_v(void)
{
    do_tool(TOOLS_VERT_FLIP, current_sprite, 0, 0);
    return D_REDRAW;
}

int tools_rot_90(void)
{
    do_tool(TOOLS_CW_ROTATE, current_sprite, 0, 0);
    return D_REDRAW;
}

int tools_rot_45(void)
{
    do_tool(TOOLS_CW_ROTATE_45, current_sprite, 0, 0);
    return D_REDRAW;
}

int tools_slide_n(void)
{
    do_tool(TOOLS_SLIDE_UP, current_sprite, 0, 0);
    return D_REDRAW;
}

int tools_slide_s(void)
{
    do_tool(TOOLS_SLIDE_DOWN, current_sprite, 0, 0);
    return D_REDRAW;
}

int tools_slide_e(void)
{
    do_tool(TOOLS_SLIDE_RIGHT, current_sprite, 0, 0);
    return D_REDRAW;
}

int tools_slide_w(void)
{
    do_tool(TOOLS_SLIDE_LEFT, current_sprite, 0, 0);
    return D_REDRAW;
}

int tools_wrap_n(void)
{
    do_tool(TOOLS_WRAP_UP, current_sprite, 0, 0);
    return D_REDRAW;
}

int tools_wrap_s(void)
{
    do_tool(TOOLS_WRAP_DOWN, current_sprite, 0, 0);
    return D_REDRAW;
}

int tools_wrap_e(void)
{
    do_tool(TOOLS_WRAP_RIGHT, current_sprite, 0, 0);
    return D_REDRAW;
}

int tools_wrap_w(void)
{
    do_tool(TOOLS_WRAP_LEFT, current_sprite, 0, 0);
    return D_REDRAW;
}

///////////  The Tools Menu 

/*  ** old menus **
MENU rotation_tools_menu[] =
{
    {"&x Flip Horiz", 	tools_flip_h, NULL, 0, NULL},
    {"&y Flip Vert", 	tools_flip_v, NULL, 0, NULL},
    {"&z Rotate 90", 	tools_rot_90, NULL, 0, NULL},
    {"&w Rotate 45", 	tools_rot_45, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL},
};

MENU move_tools_menu[] =
{
    {"&h Slide Left", 	tools_slide_w, NULL, 0, NULL},
    {"&j Slide Down", 	tools_slide_s, NULL, 0, NULL},
    {"&k Slide Up", 	tools_slide_n, NULL, 0, NULL},
    {"&l Slide Right", 	tools_slide_e, NULL, 0, NULL},
    {"",		NULL, NULL, 0, NULL},
    {"&u Wrap Left", 	tools_wrap_w, NULL, 0, NULL},
    {"&i Wrap Down", 	tools_wrap_s, NULL, 0, NULL},
    {"&o Wrap Up", 	tools_wrap_n, NULL, 0, NULL},
    {"&p Wrap Right", 	tools_wrap_e, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL},
};

MENU tools_menu[] =
{
    {"&p Paint",         edit_mode_paint, NULL, 0, NULL},
    {"&f Flood Fill",    edit_mode_flood, NULL, 0, NULL},
    {"&e Eyedrop", 	 edit_mode_eye, NULL, 0, NULL},
    {"",		NULL, NULL, 0, NULL},
    {"&k Clear", 	 tools_clear_bg, NULL, 0, NULL},
    {"",		NULL, NULL, 0, NULL},
    {"&Rotation >",	NULL, rotation_tools_menu, 0, NULL},
    {"&Move     >",	NULL, move_tools_menu, 0, NULL},
    {NULL, NULL, NULL, 0, NULL},
};
*/

MENU tools_menu[] =
{
    {"&k Clear", 	 tools_clear_bg, NULL, 0, NULL},
    {"",		NULL, NULL, 0, NULL},
    {"&x Flip Horiz", 	tools_flip_h, NULL, 0, NULL},
    {"&y Flip Vert", 	tools_flip_v, NULL, 0, NULL},
    {"&z Rotate 90", 	tools_rot_90, NULL, 0, NULL},
    {"&w Rotate 45", 	tools_rot_45, NULL, 0, NULL},
    /*  eh.. screw these... noone ever uses 'em anyway...
    {"",		NULL, NULL, 0, NULL},
    {"&h Slide Left", 	tools_slide_w, NULL, 0, NULL},
    {"&j Slide Down", 	tools_slide_s, NULL, 0, NULL},
    {"&k Slide Up", 	tools_slide_n, NULL, 0, NULL},
    {"&l Slide Right", 	tools_slide_e, NULL, 0, NULL},
    */
    {"",		NULL, NULL, 0, NULL},
    {"&u Roll Left", 	tools_wrap_w, NULL, 0, NULL},
    {"&i Roll Down", 	tools_wrap_s, NULL, 0, NULL},
    {"&o Roll Up", 	tools_wrap_n, NULL, 0, NULL},
    {"&p Roll Right", 	tools_wrap_e, NULL, 0, NULL},
    {"",		NULL, NULL, 0, NULL},
    {"&c Conway's Life", do_life, NULL, 0, NULL},
    {"",		NULL, NULL, 0, NULL},
    {"&a Animate",      animate_toggle, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL},
};

