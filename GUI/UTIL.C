// util.c
//
//  bitmap editor UI module for allegro
//
//  September, 1998
//  jerry@mail.csh.rit.edu

#include <stdio.h>
#include "../INCLUDE/ALLEGRO.H"
#include "../INCLUDE/general.h" // for the color definitions
#include "../INCLUDE/util.h"

void position_dialog(DIALOG* dialog, int x, int y) {
    if (!dialog) return;

    // Find current position of first element to calculate offset
    int current_x = dialog[0].x;
    int current_y = dialog[0].y;
    int dx = x - current_x;
    int dy = y - current_y;

    // Move all dialog elements
    for (int i = 0; dialog[i].proc != NULL; i++) {
        dialog[i].x += dx;
        dialog[i].y += dy;
    }
}

void my_draw_dotted_rect(MYBITMAP *bmp,int x1,int y1,int x2,int y2,int c)
{
    int x, y;

    for(x=x1; x<=x2; x+=2)
	putpixel(bmp,x,y1,c);

    for(x=x1+1; x<=x2; x+=2)
	putpixel(bmp,x,y2,c);

    for(y=y1; y<=y2; y+=2)
	putpixel(bmp,x1,y,c);

    for(y=y1+1; y<=y2; y+=2)
	putpixel(bmp,x2,y,c);
}


#ifndef USE_DEGUI
void draw_dotted_rect(MYBITMAP *bmp,int x1,int y1,int x2,int y2,int c)
{
    my_draw_dotted_rect(bmp, x1, y1, x2, y2, c);
}
#endif

void border_3d(MYBITMAP * bmp, int x, int y, int w, int h, int up)
{
    int ca,cb;

    if (up)
    {
	ca = GUI_L_SHAD;
	cb = GUI_D_SHAD;
    } else {
	ca = GUI_D_SHAD;
	cb = GUI_L_SHAD;
    }

    line(bmp, x, y, x+w-1, y, ca);
    line(bmp, x, y, x, y+h-1, ca);
    line(bmp, x+1, y+h-1, x+w-1, y+h-1, cb);
    line(bmp, x+w-1, y+1, x+w-1, y+h-1, cb);
}

void box_3d(MYBITMAP * bmp, 
	    int w, int h, 
	    int up, int selected)
{
    int ca,cb;

    if (up)
    {
	ca = GUI_L_SHAD;
	cb = GUI_D_SHAD;
    } else {
	ca = GUI_D_SHAD;
	cb = GUI_L_SHAD;
    }

    rect(bmp, 0, 0, w-1, h-1, GUI_FORE);
    line(bmp, 1, 1, w-2, 1, cb);
    line(bmp, 1, 1, 1, h-2, cb);
    line(bmp, 2, h-2, w-2, h-2, ca);
    line(bmp, w-2, 2, w-2, h-2, ca);

//    rect(bmp, 1, 1, w-2, h-2, (selected)?COLOR_HILITE:COLOR_LOLITE);

    if (selected)
    {
	rect(bmp, 0, 0,  w-1, h-1, GUI_BACK);
	draw_dotted_rect(bmp, 0, 0, w-1, h-1, GUI_FORE);
    } else {
	rect(bmp, 0, 0,  w-1, h-1, GUI_FORE);
    }
}

int get_config_on_off(const char* section, const char* name, int default_value) {
    char* str_value = get_config_string(section, name, NULL);
    int result = default_value;

    if (str_value != NULL) {
        if (_stricmp(str_value, "on") == 0) {
            result = 1;
        }
        else if (_stricmp(str_value, "off") == 0) {
            result = 0;
        }
        // If it's neither "on" nor "off", keep the default
        free(str_value);
    }

    return result;
}
