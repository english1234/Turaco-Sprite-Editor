// editmode.h
//
//  paint mode functions
//
//  November 1998
//  jerry@mail.csh.rit.edu

extern MYBITMAP * cursor;
extern MYBITMAP * busy_cursor;
extern MYBITMAP * flood_cursor;
extern MYBITMAP * eye_cursor;


void Init_Cursors(void);
void DeInit_Cursors(void);

void busy(void);
void not_busy(void);
void empty_cursor(void);

int edit_mode_paint(void);
int edit_mode_flood(void);
int edit_mode_eye(void);

#define MODE_PAINT     (0)
#define MODE_FLOODFILL (1)
#define MODE_EYEDROP   (2)


void set_mouse_edit_mode(int newmode);

extern int edit_mode;
extern int edit_mode_old;


// these are macros for mouse cursor changes.
// if the order of these are changed, there might be 
// some odd behavior or wierd graphics anomalies.
#define SET_CURSOR(w,x,y) \
	show_mouse(NULL); \
	set_mouse_sprite(w);  \
	set_mouse_sprite_focus(x,y); \
	show_mouse(screen);

#define REGULAR_CURSOR() \
	SET_CURSOR(cursor, 0, 0);

#define BUSY_CURSOR() \
	SET_CURSOR(busy_cursor, 17, 5);

#define FLOOD_CURSOR() \
	SET_CURSOR(flood_cursor, 1, 15);

#define EYE_CURSOR() \
	SET_CURSOR(eye_cursor, 2, 14);
