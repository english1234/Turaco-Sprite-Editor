// util.c
//
//  Helper functions for the ui et al.
//
//  September, 1998
//  jerry@mail.csh.rit.edu

// to write out "on"/"off" into a n int.
#define set_config_on_off(s, i, v) \
    set_config_string((s), (i), (v)?"on":"off");


int get_config_on_off(const char* section, const char* name, int default_value);

// position the dialog at a specific location
void position_dialog(DIALOG *d, int x, int y);


// draw a border raised or not
void border_3d(MYBITMAP * bmp, int x, int y, int w, int h, int up);


// draw a 3d-box with selection box in the whole bitmap
void box_3d(MYBITMAP * bmp, int w, int h, 
	    int up, int selected);


// this is from the degui lib.  if it's not linked, we can't use it.
void draw_dotted_rect(MYBITMAP *bmp,int x1,int y1,int x2,int y2,int c);

// and my own one which doesn't screw up square boxes.
void my_draw_dotted_rect(MYBITMAP *bmp,int x1,int y1,int x2,int y2,int c);

/// ripped this out of degui source... it's available to us, just, but this
/// is an internal function... we'll use it anyway...

extern void _draw_progressbar (const int x, const int y, 
                               const int w, const int h,
                               const int drawtext,
			       const float progress,
			       const float scale,
			       const int disabled,
			       const int fore,
			       const int select,
			       const int deselect,
			       const int disable,
			       const int light_shad, const int dark_shad);
