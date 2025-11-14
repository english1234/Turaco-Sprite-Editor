// toolmenu.h
//
//  bitmap manipulations tools
//
//  November 1997 & September 1998
//  jerry@mail.csh.rit.edu

#include "../INCLUDE/ALLEGRO.H"

extern MENU tools_menu[];

// Manipulation Operations
enum {
    TOOLS_CLEAR_BITMAP,
    TOOLS_PAINT_FG,
    TOOLS_PAINT_BG,
    TOOLS_FLOOD_FILL_FG,
    TOOLS_FLOOD_FILL_BG,
    TOOLS_EYEDROPPER_FG,
    TOOLS_EYEDROPPER_BG,
    TOOLS_VERT_FLIP,
    TOOLS_HORIZ_FLIP,
    TOOLS_CW_ROTATE,
    TOOLS_CW_ROTATE_45,
    TOOLS_SLIDE_UP,
    TOOLS_SLIDE_DOWN,
    TOOLS_SLIDE_LEFT,
    TOOLS_SLIDE_RIGHT,
    TOOLS_SWAP_CHARS,
    TOOLS_WRAP_UP,
    TOOLS_WRAP_DOWN,
    TOOLS_WRAP_LEFT,
    TOOLS_WRAP_RIGHT,
    TOOLS_CCW_ROTATE_45,
    TOOLS_LIFE,

    MAX_TOOLS // this entry must always be last!!!
};

// bring up the manipulations requestor.  it returns one of the above values.
int ToolSelector(void);

// do the selected manipulation, on the CurrentChar, and in location x,y
// if applicable (send in 0's if not applicable) - only used in flood fill
// so far...
void do_tool(int selection, MYBITMAP * CurrentChar, int x, int y);

// used by orientation code in coding and decoding romsets
void tools_flip(MYBITMAP * CurChar, int direction);
void tools_cw_rotate(MYBITMAP * CurChar);
