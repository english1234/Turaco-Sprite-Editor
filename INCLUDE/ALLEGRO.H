#ifndef _ALLEGRO_H
#define _ALLEGRO_H

#define _CRT_SECURE_NO_WARNINGS
#define SDL_PATH "C:\Users\robin\Documents\Vector_monitor_project\SDL2\include\SDL.h"
#include SDL_PATH

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef BOOL
typedef int BOOL;
#endif

#define D_O_K 0

#define INIT_SCREEN_W 960
#define INIT_SCREEN_H 720

// Screen dimension macros
#define SCREEN_W  (screen->w)
#define SCREEN_H  (screen->h)

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX  
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef SEND_MESSAGE
#define SEND_MESSAGE(dlg, msg, c) ((dlg)->proc((msg), (dlg), (c)))
#endif

typedef int fixed;  // Fixed point number type

// File attribute constants (from DOS/Allegro)
#define FA_RDONLY       1
#define FA_HIDDEN       2
#define FA_SYSTEM       4
#define FA_LABEL        8
#define FA_DIREC        16
#define FA_ARCH         32

#define MSG_START       1
#define MSG_END         2
#define MSG_DRAW        3
#define MSG_CLICK       4
#define MSG_DCLICK      5
#define MSG_KEY         6
#define MSG_CHAR        7
#define MSG_UCHAR       8
#define MSG_XCHAR       9
#define MSG_WANTFOCUS   10
#define MSG_GOTFOCUS    11
#define MSG_LOSTFOCUS   12
#define MSG_GOTMOUSE    13
#define MSG_LOSTMOUSE   14
#define MSG_IDLE        15
#define MSG_RADIO       16
#define MSG_WHEEL       17
#define MSG_LPRESS      18
#define MSG_LRELEASE    19
#define MSG_MPRESS      20
#define MSG_MRELEASE    21
#define MSG_RPRESS      22
#define MSG_RRELEASE    23
#define MSG_WANTMOUSE   24
#define MSG_MOUSEMOVE   25

#define MSG_USER        26

// FONT DEFINITIONS

typedef struct FONT {
    int height;
    void* data;  // Remove the vtable if Turaco doesn't use it
} FONT;

typedef struct FONT_VTABLE {
    int (*font_height)(const struct FONT* f);
    int (*char_length)(const struct FONT* f, int ch);
    int (*text_length)(const struct FONT* f, const char* text);
    int (*render_char)(const struct FONT* f, int ch, int fg, int bg, void* bmp, int x, int y);
    void (*render)(const struct FONT* f, const char* text, int fg, int bg, void* bmp, int x, int y);
    void (*destroy)(struct FONT* f);
} FONT_VTABLE;

typedef struct FONT_8x8 {
    unsigned char dat[256][8];  // Full 256 character set
} FONT_8x8;

// PCX header structure
typedef struct {
    unsigned char manufacturer;
    unsigned char version;
    unsigned char encoding;
    unsigned char bits_per_pixel;
    unsigned short xmin, ymin;
    unsigned short xmax, ymax;
    unsigned short hres, vres;
    unsigned char palette[48];
    unsigned char reserved;
    unsigned char color_planes;
    unsigned short bytes_per_line;
    unsigned short palette_type;
    unsigned char filler[58];
} PCX_HEADER;

// DIALOGUE DEFINITIONS

#define D_EXIT          1     // Object makes the dialog exit
#define D_SELECTED      2     // Object is selected
#define D_GOTFOCUS      4     // Object has the input focus
#define D_GOTMOUSE      8     // Mouse is on top of object
#define D_HIDDEN        16    // Object is not visible
#define D_DISABLED      32    // Object is visible but inactive
#define D_DIRTY         64    // Object needs to be redrawn
#define D_INTERNAL      128   // Reserved for internal use
#define D_USER          256   // From here on is free for your own use

#define D_O_K           0     // Normal return value
#define D_CLOSE         1     // Close the dialog
#define D_REDRAW        2     // Redraw the dialog
#define D_REDRAWME      4     // Redraw this object
#define D_WANTFOCUS     8     // Give this object the focus
#define D_USED_CHAR     16    // Input char was used
#define D_REDRAW_ALL    32    // Redraw all the dialog
#define D_DONTWANTMOUSE 64    // Object doesn't want mouse focus

typedef struct DIALOG {
    int (*proc)(int msg, struct DIALOG *d, int c);  // Dialog procedure function
    int x, y, w, h;     // Position and size
    int fg, bg;         // Foreground and background colors
    int key;            // Keyboard shortcut
    int flags;          // Status flags
    int d1, d2;         // Whatever you want to use them for
    void *dp, *dp2, *dp3;  // Pointers to data
    void *dp_reserved;  // Reserved pointer
} DIALOG;

// MENU DEFINITIONS

#define D_DISABLED      32    // Menu item is disabled (grayed out)
#define D_SELECTED      2     // Menu item is selected/checked

typedef struct MENU {
    char* text;                    // Menu item text
    int (*proc)(void);             // Callback function when item is selected
    struct MENU* child;            // Pointer to submenu (NULL if none)
    int flags;                     // Status flags
    void* dp;                      // Extra data pointer
} MENU;

// MYBITMAP DEFINITIONS

typedef struct MYBITMAP {
    int w, h;                    // Width and height in pixels
    int clip;                    // Clipping flag (true/false)
    int cl, cr, ct, cb;          // Clipping rectangle (left, right, top, bottom)
    void* vtable;                // Virtual function table
    void (*write_bank)(void);    // Bank switching routines
    void (*read_bank)(void);
    void* dat;                   // Pointer to the actual pixel data
    unsigned long id;            // Unique bitmap ID
    void* extra;                 // Extra info (for internal use)
    int x_ofs;                   // X offset for sub-bitmaps
    int y_ofs;                   // Y offset for sub-bitmaps
    int seg;                     // Segment for bank switching
    unsigned char** line;        // Array of pointers to each line
} MYBITMAP;

// In Allegro's default palette mode, RGB values range from 0-63, not 0-255
// like most modern systems. This is because Allegro was designed for VGA 
// hardware which used 6-bit color components.
typedef struct RGB {
    unsigned char r, g, b;    // Red, green, blue components (0-63 in Allegro, not 0-255!)
    unsigned char filler;     // Padding byte (unused, for alignment)
} RGB;

// PALETTE_SELECT structure (custom to Turaco)

typedef RGB PALETTE[256];

typedef struct palette_select {
    RGB* palette;  // palette for graphics - color 0 thru color N
    int ncolors;    // number of colors - MUST be a multiple of 2
    int basecolor;  // first palette color to base things on.
} PALETTE_SELECT;

// Mouse button flags
#define MOUSE_FLAG_LEFT_DOWN    1
#define MOUSE_FLAG_RIGHT_DOWN   2
#define MOUSE_FLAG_MIDDLE_DOWN  4

// Scancode to key conversion macro
#define SCANCODE_TO_KEY(scancode) ((scancode) << 8)

// F-key scancodes (Allegro/PC BIOS scancodes)
#define KEY_F1      59
#define KEY_F2      60
#define KEY_F3      61
#define KEY_F4      62
#define KEY_F5      63
#define KEY_F6      64
#define KEY_F7      65
#define KEY_F8      66
#define KEY_F9      67
#define KEY_F10     68
#define KEY_F11     87
#define KEY_F12     88

// Additional key scancodes referenced in your code
#define KEY_PGUP    73
#define KEY_PGDN    81
#define KEY_HOME    71
#define KEY_END     79
#define KEY_P       25
#define KEY_A       30

// Other globals that appear to be used
extern long life_counter;  // Counter for the "life" feature (possibly Conway's Game of Life?)

// Key code constants (Allegro format: scancode in high byte)
#define KEY_X       (45 << 8)
#define KEY_Y       (21 << 8)
#define KEY_Z       (44 << 8)
#define KEY_W       (17 << 8)
#define KEY_K       (37 << 8)
#define KEY_G       (34 << 8)
#define KEY_UP      (72 << 8)
#define KEY_DOWN    (80 << 8)
#define KEY_LEFT    (75 << 8)
#define KEY_RIGHT   (77 << 8)

//#define GUI_BACK   -1  // Often used with text_mode() for transparent text

// Graphics mode constants
#define GFX_TEXT           0
#define GFX_AUTODETECT     1
#define GFX_SAFE          -1



extern int d_clear_proc(int msg, DIALOG* d, int c);
extern int d_shadow_box_proc(int msg, DIALOG* d, int c);
extern int d_ctext_proc(int msg, DIALOG* d, int c);
extern int d_textbox_proc(int msg, DIALOG* d, int c);
extern int d_button_proc(int msg, DIALOG* d, int c);
extern void show_mouse(MYBITMAP* bmp);
extern void text_mode(int mode);
extern void textout_centre(MYBITMAP* bmp, FONT* font, const char* text, int x, int y, int color);
extern void stretch_blit(MYBITMAP* src, MYBITMAP* dst,
    int src_x, int src_y, int src_w, int src_h,
    int dst_x, int dst_y, int dst_w, int dst_h);
extern void broadcast_dialog_message(int message, int param);
extern void textout(MYBITMAP* bmp, FONT* font, const char* text, int x, int y, int color);
extern void textout_centre(MYBITMAP* bmp, FONT* font, const char* text, int x, int y, int color);
extern int  putpixel(MYBITMAP* bmp, int x, int y, int color);
extern int getpixel(MYBITMAP* bmp, int x, int y);
extern void hline(MYBITMAP* bmp, int x1, int y, int x2, int color);
extern void vline(MYBITMAP* bmp, int x, int y1, int y2, int color);

// System functions
extern void allegro_exit(void);
extern void install_keyboard(void);
extern void install_mouse(void);
extern void install_timer(void);
extern void install_int(void (*handler)(void), int interval);
extern int set_gfx_mode(int card, int w, int h, int v_w, int v_h);

// Palette functions
extern void set_palette(PALETTE pal);
extern void set_color(int index, RGB* color);
extern void set_palette_range(PALETTE pal, int from, int to, int retracesync);

// Drawing functions
extern void clear(MYBITMAP* bmp);
extern void line(MYBITMAP* bmp, int x1, int y1, int x2, int y2, int color);
extern void rect(MYBITMAP* bmp, int x1, int y1, int x2, int y2, int color);
extern void floodfill(MYBITMAP* bmp, int x, int y, int color);

// Sprite functions
extern void draw_sprite_h_flip(MYBITMAP* bmp, MYBITMAP* sprite, int x, int y);
extern void draw_sprite_v_flip(MYBITMAP* bmp, MYBITMAP* sprite, int x, int y);
extern void draw_sprite_vh_flip(MYBITMAP* bmp, MYBITMAP* sprite, int x, int y);
extern void rotate_sprite(MYBITMAP* bmp, MYBITMAP* sprite, int x, int y, fixed angle);

// Dialog functions
extern void centre_dialog(DIALOG* dialog);
extern void set_dialog_color(DIALOG* dialog, int fg, int bg);
extern int do_menu(MENU* menu, int x, int y);

// Dialog procedures
extern int d_edit_proc(int msg, DIALOG* d, int c);
extern int d_list_proc(int msg, DIALOG* d, int c);
extern int d_slider_proc(int msg, DIALOG* d, int c);
extern int d_text_proc(int msg, DIALOG* d, int c);

// File functions
extern int exists(const char* filename);
extern int file_select(const char* title, char* path, const char* ext);
extern long file_size(const char* filename);
extern char* get_filename(const char* path);
extern void put_backslash(char* path);

// Bitmap functions
extern MYBITMAP* load_bitmap(const char* filename, PALETTE pal);
extern MYBITMAP* load_pcx(const char* filename, PALETTE pal);
extern void save_pcx(const char* filename, MYBITMAP* bmp, PALETTE pal);

// Configuration functions
extern void push_config_state(void);
extern void pop_config_state(void);
extern void set_config_file(const char* filename);
extern void set_config_string(const char* section, const char* name, const char* value);
extern void set_config_int(const char* section, const char* name, int value);
extern char* get_config_string(const char* section, const char* name, const char* default_value);
extern int get_config_int(const char* section, const char* name, int default_value);
extern char** get_config_argv(const char* section, const char* name, int* argc);

// Input functions
extern int keypressed(void);
extern int readkey(void);
extern void simulate_keypress(int key);
extern int sdl_key_to_allegro(SDL_Keysym keysym);

// Text functions
extern void textprintf(MYBITMAP* bmp, FONT* font, int x, int y, int color, const char* format, ...);
extern void textprintf_centre(MYBITMAP* bmp, FONT* font, int x, int y, int color, const char* format, ...);
extern int text_height(FONT* font);

// Mouse functions
extern void set_mouse_sprite(MYBITMAP* sprite);
extern void set_mouse_sprite_focus(int x, int y);
extern void set_mouse_range(int x1, int y1, int x2, int y2);
extern void update_mouse_state(SDL_Event* event);

// Other functions
extern void fade_out(int speed);
extern fixed itofix(int x);
extern void LOCK_FUNCTION(void *);
extern void LOCK_VARIABLE(int);
extern void SDL_Flip(void);

// In allegro.h
extern int gfx_hres, gfx_vres; // Make sure these are declared


// Macros for backward compatibility
#define textprintf(bmp, font, x, y, color, ...) textprintf_ex(bmp, font, x, y, color, 0, __VA_ARGS__)
#define textprintf_centre(bmp, font, x, y, color, ...) textprintf_ex(bmp, font, x, y, color, 1, __VA_ARGS__)

#define FLIP_H 0
#define FLIP_V 1
#define FLIP_VH 2

#define draw_sprite(bmp, sprite, x, y) draw_sprite_ex(bmp, sprite, x, y, 0)
#define draw_sprite_h_flip(bmp, sprite, x, y) draw_sprite_ex(bmp, sprite, x, y, FLIP_H)
#define draw_sprite_v_flip(bmp, sprite, x, y) draw_sprite_ex(bmp, sprite, x, y, FLIP_V)
#define draw_sprite_vh_flip(bmp, sprite, x, y) draw_sprite_ex(bmp, sprite, x, y, FLIP_VH)

#endif