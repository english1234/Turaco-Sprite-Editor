// main.c

//
//  The main application file..
//
//  September, 1998
//  jerry@mail.csh.rit.edu

char title_text[] = "Turaco v1.1.3";
char title_date[] = "07 Oct 1999";

#include "../INCLUDE/allegro.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h> // for strncpy
#include <time.h>
#include <conio.h>

#include "../INCLUDE/general.h"
#include "../INCLUDE/sprtplte.h"
#include "../INCLUDE/guipal.h"
#include "../INCLUDE/palette.h"
#include "../INCLUDE/config.h"
#include "../INCLUDE/editmenu.h"
#include "../INCLUDE/gamedesc.h"
#include "../INCLUDE/inidriv.h"
#include "../INCLUDE/sprite.h"
#include "../INCLUDE/list.h"
#include "../INCLUDE/editmode.h"
#include "../INCLUDE/toolmenu.h"
#include "../INCLUDE/BITMAPED.H"
#include "../INCLUDE/UTIL.H"

#include SDL_PATH

// Global mouse state variables
volatile int mouse_x = 0;
volatile int mouse_y = 0;
volatile int mouse_z = 0;
volatile int mouse_b = 0;

// Add these after your other global variables
PALETTE desktop_palette;
int gui_fg_color = 0;
int gui_bg_color = 0;
int gui_mg_color = 0;

int i_love_bill = 0;

char command_line_driver[127]; // if a driver was named on the command line...

int gfx_hres = INIT_SCREEN_W;
int gfx_vres = INIT_SCREEN_H; // 480 also works well..

volatile int timer_ticks = 0;

MYBITMAP* screen = NULL;
MYBITMAP* current_sprite = NULL;
FONT my_new_font;
FONT* font = NULL;
FONT* original_font = NULL;

// Global SDL variables
SDL_Window* sdl_window = NULL;
SDL_Renderer* sdl_renderer = NULL;
SDL_Texture* sdl_texture = NULL;
Uint32* sdl_pixels = NULL;

extern DIALOG main_dialog[];  // Forward declaration
extern int palette_dirty;
extern int text_background_mode;
extern FONT_8x8 builtin_font;

void usage(void)
{
    printf("\nUsage:\n");
    printf("    TURACO.EXE [ game | -help | -v ]\n");
    printf("\t game     \tload the specified game\n");
    printf("\t -help    \tdisplay this information\n");
    printf("\t -v       \tdisplay version information\n");
    printf("\n");
    printf("    TURACO.EXE [ -list | -listhtml | -listfull | -listroms ]\n");
    printf("\t -list    \tlist all of the available drivers\n");
    printf("\t -listhtml\tlist all of the available drivers in html format\n");
    printf("\t -listfull\tlist all of the drivers with extra info\n");
    printf("\t -listroms\tlist all of the drivers' required roms\n");
}

void version_info(void)
{
    printf("\n");
    printf("%s  (%s)\n", title_text, title_date);
    //printf("\tTuraco.ini version %d.%d\n", 
    //		ini_version_major, ini_version_minor);
    printf("\tBuilt: %s, %s using:\n", __DATE__, __TIME__);
    //  printf("\t       Allegro v%s (%s)\n", ALLEGRO_VERSION_STR, ALLEGRO_DATE_STR);
#ifdef USE_DEGUI
    printf("\t       DEGUI v%s (%d)\n", DEGUI_VERSION_STR, DEGUI_DATE);
#endif
    //  printf("\t       DJGPP v%s\n", __VERSION__);
}

#define RET_DOIT     (0)
#define RET_VERSION  (1)
#define RET_USAGE    (2)
#define RET_LIST     (3)
#define RET_LISTFULL (4)
#define RET_LISTHTML (5)
#define RET_LISTROMS (6)

int parse_command_line(int argc, char** argv)
{
    // return 0 to go into graphics mode, 
    // return 1 to exit immediately

    if (argc == 1)  return RET_DOIT;
    if (argc > 2)   return RET_USAGE;

    command_line_driver[0] = '\0';

    if (argv[1][0] == '-')   // it wants to be a command line option
    {
        if (!strcmp(argv[1], "--version"))  return RET_VERSION;
        if (!strcmp(argv[1], "-v"))         return RET_VERSION;

        if (!strcmp(argv[1], "-help"))  return RET_USAGE;
        if (!strcmp(argv[1], "-h"))     return RET_USAGE;
        if (!strcmp(argv[1], "-?"))     return RET_USAGE;

        if (!strcmp(argv[1], "-list"))      return RET_LIST;
        if (!strcmp(argv[1], "-listfull"))  return RET_LISTFULL;
        if (!strcmp(argv[1], "-listhtml"))  return RET_LISTHTML;
        if (!strcmp(argv[1], "-listroms"))  return RET_LISTROMS;

        return RET_USAGE;
    }
    else
        strncpy(command_line_driver, argv[1], 126);

    return RET_DOIT;
}

void end_blurb(void)
{
    printf("%s  (%s)\n\n", title_text, title_date);
    printf("A re-write of \"AGE\", The Arcade Games Editor\n\n");
    printf("Written by:\n");
    printf("\tScott \"Jerry\" Lawrence  jerry@mail.csh.rit.edu\n");
    printf("\tIvan Mackintosh         ivan@rcp.co.uk\n");

    printf("\nSpecial thanks to:\n");
    printf("\tChris \"Zwaxy\" Moore, M.C. Silvius, David Caldwell\n");

    printf("\tAll of those involved with MAME, and other emulators,\n");
    printf("\tAll of those who wrote the original games.\n");
    printf("\n\t\t\t   THANKS!\n");
    printf("\n");
    printf("Need help?  Have suggestions?  Want the latest version?  Want\n");
    printf("to ask us something?  Go to the message board, and homepage at:\n");
    printf("\n");
    printf("\thttp://www.csh.rit.edu/~jerry/turaco/\n");
    printf("\n");
}

void setup_palette(void)
{
    // Make sure desktop_palette is properly initialized
    if (!desktop_palette) {
        printf("ERROR: desktop_palette is NULL!\n");
        return;
    }

    // Initialize all to black first
    for (int i = 0; i < 256; i++) {
        desktop_palette[i].r = 0;
        desktop_palette[i].g = 0;
        desktop_palette[i].b = 0;
    }

    // Set up GUI colors (indices 1-8)
    // These MUST match the GUI_* defines in general.h

    desktop_palette[1].r = 0;   desktop_palette[1].g = 0;   desktop_palette[1].b = 0;    // GUI_FORE - Black (text)
    desktop_palette[2].r = 32;  desktop_palette[2].g = 32;  desktop_palette[2].b = 32;   // GUI_MID - Medium grey (borders)
    desktop_palette[3].r = 48;  desktop_palette[3].g = 48;  desktop_palette[3].b = 48;   // GUI_BACK - Light grey (background)
    desktop_palette[4].r = 56;  desktop_palette[4].g = 56;  desktop_palette[4].b = 56;   // GUI_L_SHAD - Light shadow
    desktop_palette[5].r = 24;  desktop_palette[5].g = 24;  desktop_palette[5].b = 24;   // GUI_D_SHAD - Dark shadow
    desktop_palette[6].r = 0;   desktop_palette[6].g = 0;   desktop_palette[6].b = 63;   // GUI_SELECT - Blue (selected items)
    desktop_palette[7].r = 48;  desktop_palette[7].g = 48;  desktop_palette[7].b = 48;   // GUI_DESELECT - Light grey
    desktop_palette[8].r = 32;  desktop_palette[8].g = 32;  desktop_palette[8].b = 32;   // GUI_DISABLE - Medium grey (disabled)

    // Set up general grayscale palette (indices 9-15)
    desktop_palette[9].r = 63;  desktop_palette[9].g = 0;   desktop_palette[9].b = 0;    // Bright Red
    desktop_palette[10].r = 0;  desktop_palette[10].g = 63; desktop_palette[10].b = 0;   // Bright Green
    desktop_palette[11].r = 0;  desktop_palette[11].g = 0;  desktop_palette[11].b = 63;  // Bright Blue
    desktop_palette[12].r = 63; desktop_palette[12].g = 63; desktop_palette[12].b = 0;   // Yellow
    desktop_palette[13].r = 63; desktop_palette[13].g = 0;  desktop_palette[13].b = 63;  // Magenta
    desktop_palette[14].r = 0;  desktop_palette[14].g = 63; desktop_palette[14].b = 63;  // Cyan
    desktop_palette[15].r = 63; desktop_palette[15].g = 63; desktop_palette[15].b = 63;  // White (bright)

    printf("GUI palette setup:\n");
    printf("  GUI_FORE (1)     = Black (text)\n");
    printf("  GUI_MID (2)      = Medium grey (borders)\n");
    printf("  GUI_BACK (3)     = Light grey (background)\n");
    printf("  GUI_L_SHAD (4)   = Light shadow\n");
    printf("  GUI_D_SHAD (5)   = Dark shadow\n");
    printf("  GUI_SELECT (6)   = Blue (selected)\n");
    printf("  GUI_DESELECT (7) = Light grey\n");
    printf("  GUI_DISABLE (8)  = Medium grey (disabled)\n");

    palette_dirty = 1;
}

void timer_handler(void)
{
    timer_ticks++;
}
END_OF_FUNCTION(timer_handler);

void force_font_initialization(void) {
    printf("Force initializing font...\n");

    if (font == NULL) {
        printf("Font is NULL - creating default font\n");

        font = malloc(sizeof(FONT));
        if (!font) { printf("ERROR: Failed to allocate font structure\n"); return; }

        font->data = malloc(sizeof(FONT_8x8));
        if (!font->data) {
            printf("ERROR: Failed to allocate font data\n");
            free(font);
            font = NULL;
            return;
        }

        font->height = 8;

        FONT_8x8* font_data = (FONT_8x8*)font->data;

        for (int c = 0; c < 256; c++)
            memcpy(font_data->dat[c], builtin_font.dat[c], 8);

        printf("Default font created successfully: height=%d, data=%p\n", font->height, font->data);
    }
    else {
        printf("Font already exists: height=%d, data=%p\n", font->height, font->data);
    }
}

void test_direct_pixel_access(void)
{
    printf("=== DIRECT PIXEL ACCESS TEST ===\n");

    if (!current_sprite) {
        printf("Creating test sprite...\n");
        current_sprite = create_bitmap(16, 16);
        if (!current_sprite) {
            printf("Failed to create test sprite!\n");
            return;
        }
        clear_to_color(current_sprite, Get_BG_Color());
    }

    // Test 1: Direct pixel setting
    printf("Test 1: Direct putpixel/getpixel\n");
    putpixel(current_sprite, 5, 5, Get_FG_Color());
    int color1 = getpixel(current_sprite, 5, 5);
    printf("Pixel at (5,5) = %d (expected: %d) %s\n",
        color1, Get_FG_Color(),
        (color1 == Get_FG_Color()) ? "PASS" : "FAIL");

    // Test 2: Using tools_paint
    printf("Test 2: Using tools_paint\n");
    tools_paint(current_sprite, 6, 6, TOOLS_PAINT_FG);

    // Test 3: Verify the pixels are different
    int color5_5 = getpixel(current_sprite, 5, 5);
    int color6_6 = getpixel(current_sprite, 6, 6);
    printf("Pixel at (5,5) = %d, at (6,6) = %d\n", color5_5, color6_6);

    printf("=== END TEST ===\n");
}

// initialize all of the non-allegro, TURACO subsystems
void Init_Subsystems(void)
{
    font = &my_new_font;

    printf("DEBUG: Initializing fonts\n");
    printf("DEBUG: Initial font pointer: %p\n", font);

    original_font = font;   // <- assign AFTER the font is ready
    printf("DEBUG: original_font set to: %p\n", original_font);

    Init_INI();

    // DEBUG: Check ROMPath value
    printf("=== ROM PATH DEBUG ===\n");
    printf("ROMPath = '%s'\n", ROMPath);
    if (strlen(ROMPath) == 0) {
        printf("WARNING: ROMPath is empty! Setting to default.\n");
        strncpy(ROMPath, DEFAULT_PATH, ROM_PATH_LEN - 1);
        ROMPath[ROM_PATH_LEN - 1] = '\0';
        printf("ROMPath now = '%s'\n", ROMPath);
    }
    printf("======================\n");

    Init_Palette();
    InitialiseGameDesc();
    Init_Cursors();

    // now some timer stuff...
    i_love_bill = TRUE;
    install_timer();
    install_int(&timer_handler, 100);

    LOCK_VARIABLE(timer_ticks);
    LOCK_FUNCTION(timer_handler);

    test_direct_pixel_access();
}


// de-initialize all of the non-allegro, TURACO subsystems
void DeInit_Subsystems(void)
{
    Save_INI();
    DeInit_INI();
    FreeDriver();
    DeInit_Palette();
    DeInit_Cursors();

    if (clipboard != NULL) destroy_bitmap(clipboard);
}

int setup_gfx_mode(void)
{
    // try to set the resolution to the mode as selected in the 
    // ini file.  if it fails, fall back on 320x240

    // check for minimum resolution (320x240)
    if (gfx_hres < 320 || gfx_vres < 240)
    {
        printf("Video resolution must be at least 320x240!\n");
        gfx_hres = 320;
        gfx_vres = 240;
    }

    if (set_gfx_mode(GFX_AUTODETECT, gfx_hres, gfx_vres, 0, 0) < 0)
    {
        // ACK! it failed... fall back on 320x240 (known working on all systems)
        gfx_hres = 320;
        gfx_vres = 240;
        if (set_gfx_mode(GFX_AUTODETECT, gfx_hres, gfx_vres, 0, 0) < 0)
            return 1; // major VGA failure... can't do anything...
    }
    return 0;
}

// Menu bar processor
// Global variables for menu state
static int active_menu = -1;
static int menu_bar_hover = -1;

int d_menu_proc(int msg, DIALOG* d, int c) {
    MENU* menu = (MENU*)d->dp;
    static int menu_bar_hover = -1;

    // No scaling calculations needed!
    const int menu_bar_height = 16;
    const int item_padding = 8;
    const int text_padding = 4;

    if (msg == MSG_DRAW) {
        // Draw at original coordinates
        rectfill(screen, d->x, d->y, d->x + d->w - 1, d->y + menu_bar_height - 1, d->bg);
        rect(screen, d->x, d->y, d->x + d->w - 1, d->y + menu_bar_height - 1, d->fg);

        // Draw each menu item at original size
        int x = d->x;
        for (int i = 0; menu[i].text != NULL; i++) {
            const char* text = menu[i].text;
            char display_text[256];
            int display_pos = 0;
            int underline_pos = -1;

            // Parse text (remove '&')
            for (int j = 0; text[j] != '\0' && display_pos < 255; j++) {
                if (text[j] == '&') {
                    underline_pos = display_pos;
                }
                else {
                    display_text[display_pos++] = text[j];
                }
            }
            display_text[display_pos] = '\0';

            // Calculate item width at original size
            int item_width = display_pos * 8 + item_padding;
            int text_x = x + text_padding;
            int text_y = d->y + (menu_bar_height - 8) / 2;

            // Highlight if hovered
            if (i == menu_bar_hover) {
                rectfill(screen, x, d->y, x + item_width - 1, d->y + menu_bar_height - 1, 1);
                textout(screen, font, display_text, text_x, text_y, 15);
            }
            else {
                textout(screen, font, display_text, text_x, text_y, d->fg);
            }

            // Draw underline for shortcut key
            if (underline_pos >= 0) {
                int underline_x = text_x + (underline_pos * 8);
                int underline_y = text_y + 8;
                hline(screen, underline_x, underline_y, underline_x + 7,
                    (i == menu_bar_hover) ? 15 : d->fg);
            }

            x += item_width;
        }
        return D_O_K;
    }

    if (msg == MSG_IDLE) {
        // Check if mouse is over menu bar
        if (mouse_y >= d->y && mouse_y < d->y + menu_bar_height) {
            int x = d->x;
            int new_hover = -1;

            for (int i = 0; menu[i].text != NULL; i++) {
                const char* text = menu[i].text;
                int display_len = 0;
                for (int j = 0; text[j] != '\0'; j++) {
                    if (text[j] != '&') display_len++;
                }
                int item_width = display_len * 8 + item_padding;

                if (mouse_x >= x && mouse_x < x + item_width) {
                    new_hover = i;
                    break;
                }
                x += item_width;
            }

            if (new_hover != menu_bar_hover) {
                menu_bar_hover = new_hover;
                return D_REDRAW;
            }
        }
        else if (menu_bar_hover != -1) {
            menu_bar_hover = -1;
            return D_REDRAW;
        }

        return D_O_K;
    }

    if (msg == MSG_CLICK) {
        printf("Menu click at (%d, %d)\n", mouse_x, mouse_y);

        // Check if click is on menu bar
        if (mouse_y >= d->y && mouse_y < d->y + menu_bar_height) {
            int x = d->x;

            for (int i = 0; menu[i].text != NULL; i++) {
                const char* text = menu[i].text;
                int display_len = 0;
                for (int j = 0; text[j] != '\0'; j++) {
                    if (text[j] != '&') display_len++;
                }
                int item_width = display_len * 8 + item_padding;

                if (mouse_x >= x && mouse_x < x + item_width) {
                    printf("Clicked on menu: %s\n", text);

                    if (menu[i].child) {
                        // Show dropdown using modal loop
                        int selected = show_dropdown_menu(menu[i].child, x, d->y + menu_bar_height);

                        printf("Menu index i=%d, d->dp=%p, menu=%p\n", i, d->dp, menu);
                        printf("menu[%d].child=%p\n", i, menu[i].child);
                        printf("Checking menu[%d].child[%d]:\n", i, selected);
                        printf("  text=%p (%s)\n", menu[i].child[selected].text,
                            menu[i].child[selected].text ? menu[i].child[selected].text : "NULL");
                        printf("  proc=%p\n", (void*)menu[i].child[selected].proc);


                        if (selected >= 0 && menu[i].child[selected].proc) {
                            printf("Executing submenu item %d\n", selected);
                            printf("Menu text: '%s'\n", menu[i].child[selected].text ? menu[i].child[selected].text : "NULL");
                            printf("Proc pointer: %p\n", (void*)menu[i].child[selected].proc);

                            // Try to read the first byte of the function to see if it's valid code
                            unsigned char* func_ptr = (unsigned char*)menu[i].child[selected].proc;
                            printf("First 4 bytes at proc address: %02X %02X %02X %02X\n",
                                func_ptr[0], func_ptr[1], func_ptr[2], func_ptr[3]);

                            // Also print the actual address of help_general for comparison
                            extern int help_general(void);
                            printf("Actual help_general address: %p\n", (void*)help_general);


                            int result = menu[i].child[selected].proc();
                            printf("Submenu proc returned: %d\n", result);
                            return result;
                        }
                        return D_REDRAW;
                    }
                    else if (menu[i].proc) {
                        // Execute menu action directly
                        printf("Executing menu action\n");
                        int result = menu[i].proc();
                        return result;
                    }
                    break;
                }
                x += item_width;
            }
        }
    }

    return D_O_K;
}

int show_dropdown_menu(MENU* submenu, int x, int y) {
    if (!submenu) return -1;

    printf("show_dropdown_menu called at (%d,%d)\n", x, y);

    // Calculate dropdown dimensions at original size (no scaling)
    int dropdown_width = 0;
    int dropdown_height = 0;

    MENU* current = submenu;
    while (current->text != NULL) {
        if (strlen(current->text) == 0) {
            dropdown_height += 5;  // No scaling
            current++;
            continue;
        }

        const char* text = current->text;
        int display_len = 0;
        for (int j = 0; text[j] != '\0'; j++) {
            if (text[j] != '&') display_len++;
        }

        int text_width = display_len * 8;  // No scaling
        if (text_width > dropdown_width) dropdown_width = text_width;
        dropdown_height += 16;  // No scaling
        current++;
    }

    dropdown_width += 48;
    dropdown_height += 16;

    // Ensure dropdown stays on screen (using original 320x240 bounds)
    if (x + dropdown_width > 320) x = 320 - dropdown_width;
    if (y + dropdown_height > 240) y = 240 - dropdown_height;
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    // Save area under menu
    MYBITMAP* saved_area = create_bitmap(dropdown_width, dropdown_height);
    if (saved_area) {
        blit(screen, saved_area, x, y, 0, 0, dropdown_width, dropdown_height);
    }

    // Menu interaction loop
    int selected_index = -1;
    int menu_active = 1;
    int hover_item = -1;

    while (menu_active) {
        // Redraw dropdown every frame
        rectfill(screen, x, y, x + dropdown_width - 1, y + dropdown_height - 1, GUI_BACK);
        rect(screen, x, y, x + dropdown_width - 1, y + dropdown_height - 1, GUI_FORE);

        // Draw shadow
        hline(screen, x + 2, y + dropdown_height, x + dropdown_width, 8);
        vline(screen, x + dropdown_width, y + 2, y + dropdown_height, 8);

        // Calculate hover item (mouse coordinates are automatically scaled by SDL)
        int new_hover = -1;
        if (mouse_x >= x && mouse_x < x + dropdown_width &&
            mouse_y >= y && mouse_y < y + dropdown_height) {

            int item_y = y + 8;
            int item_index = 0;
            current = submenu;

            while (current->text != NULL) {
                if (strlen(current->text) == 0) {
                    item_y += 5;  // No scaling
                    current++;
                    continue;
                }

                if (mouse_y >= item_y && mouse_y < item_y + 16) {  // No scaling
                    if (!(current->flags & D_DISABLED)) {
                        // Use array index (includes separators) not display index
                        new_hover = (int)(current - submenu);
                    }
                    break;
                }

                item_y += 16;  // No scaling
                item_index++;
                current++;
            }
        }
        hover_item = new_hover;

        // Draw menu items
        current = submenu;
        int item_y = y + 8;
        int item_index = 0;

        while (current->text != NULL) {
            // Handle separators
            if (strlen(current->text) == 0) {
                int sep_y = item_y + 2;
                hline(screen, x + 8, sep_y, x + dropdown_width - 8, 8);
                item_y += 5;  // No scaling
                current++;
                continue;
            }

            int item_color = (current->flags & D_DISABLED) ? 8 : 0;

            // Highlight hovered item (compare array indices)
            int current_array_index = (int)(current - submenu);
            if (current_array_index == hover_item) {
                rectfill(screen, x + 4, item_y,
                    x + dropdown_width - 5, item_y + 15, GUI_SELECT);  // Fixed height: 16
                item_color = 15;
            }

            // Parse text
            char display_text[256];
            int display_pos = 0;
            int underline_pos = -1;

            for (int j = 0; current->text[j] != '\0' && display_pos < 255; j++) {
                if (current->text[j] == '&') {
                    underline_pos = display_pos;
                }
                else {
                    display_text[display_pos++] = current->text[j];
                }
            }
            display_text[display_pos] = '\0';

            // Draw checkmark if selected
            if (current->flags & D_SELECTED) {
                textout(screen, font, "v", x + 12, item_y + 4, item_color);
            }

            // Draw text (no scaling)
            textout(screen, font, display_text, x + 32, item_y + 4, item_color);

            // Draw underline (no scaling)
            if (underline_pos >= 0) {
                int underline_x = x + 32 + (underline_pos * 8);  // No scaling
                int underline_y = item_y + 12;  // Fixed position
                hline(screen, underline_x, underline_y, underline_x + 7, item_color);
            }

            // Draw submenu arrow
            if (current->child != NULL) {
                textout(screen, font, ">", x + dropdown_width - 24, item_y + 4, item_color);
            }

            item_y += 16;  // No scaling
            item_index++;
            current++;
        }

        // Update screen
        SDL_Flip();

        // Process events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            update_mouse_state(&event);

            switch (event.type) {
            case SDL_QUIT:
                menu_active = 0;
                selected_index = -1;
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    // Check if click on item
                    if (mouse_x >= x && mouse_x < x + dropdown_width &&
                        mouse_y >= y && mouse_y < y + dropdown_height) {

                        int item_y = y + 8;
                        int item_index = 0;
                        current = submenu;

                        while (current->text != NULL) {
                            if (strlen(current->text) == 0) {
                                item_y += 5;  // No scaling
                                current++;
                                continue;
                            }

                            if (mouse_y >= item_y && mouse_y < item_y + 16) {  // No scaling
                                if (!(current->flags & D_DISABLED)) {
                                    // Use array index (includes separators) not display index
                                    selected_index = (int)(current - submenu);
                                    menu_active = 0;
                                    printf("Menu item %d selected (array index)\n", selected_index);
                                }
                                break;
                            }

                            item_y += 16;  // No scaling
                            item_index++;
                            current++;
                        }
                    }
                    else {
                        // Click outside - close menu
                        printf("Click outside menu\n");
                        menu_active = 0;
                        selected_index = -1;
                    }
                }
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    menu_active = 0;
                    selected_index = -1;
                }
                break;
            }
        }

        SDL_Delay(10);
    }

    // Restore screen
    if (saved_area) {
        blit(saved_area, screen, 0, 0, x, y, dropdown_width, dropdown_height);
        destroy_bitmap(saved_area);
    }

    SDL_Flip();

    printf("Menu closed, selection: %d\n", selected_index);
    return selected_index;
}

// Keyboard shortcut processor
int d_keyboard_proc(int msg, DIALOG* d, int c)
{
    if (msg == MSG_CHAR || msg == MSG_XCHAR) {
        // Check if the key matches
        if ((c & 0xFF) == d->key || (c >> 8) == d->key) {
            // Call the callback function
            if (d->dp) {
                int (*proc)(void) = (int (*)(void))d->dp;
                int ret = (*proc)();
                if (ret == D_REDRAW) {
                    return D_REDRAW;
                }
                return D_USED_CHAR;
            }
        }
    }
    return D_O_K;
}

int do_dialog(DIALOG* dialog, int focus_obj) {
    SDL_Event event;
    int running = 1;
    int result = 0;
    Uint32 last_render_time = SDL_GetTicks();
    const Uint32 min_frame_time = 16; // ~60 FPS

    // Reset mouse range to full screen when starting a new dialog
    set_mouse_range(0, 0, SCREEN_W - 1, SCREEN_H - 1);

    // Verify menu integrity
    extern MENU help_menu[];

    // Send MSG_START to all dialog objects
    for (int i = 0; dialog[i].proc != NULL; i++) {
        // printf("Initializing dialog object %d\n", i);
        dialog[i].proc(MSG_START, &dialog[i], 0);
    }

    // Draw initial state
    for (int i = 0; dialog[i].proc != NULL; i++) {
        dialog[i].proc(MSG_DRAW, &dialog[i], 0);
    }

    SDL_Flip();

    // Main event and render loop
    while (running) {
        Uint32 current_time = SDL_GetTicks();

        // Process all pending events
        while (SDL_PollEvent(&event)) {
            // Update mouse state based on SDL event
            update_mouse_state(&event);

            switch (event.type) {
            case SDL_QUIT:
                printf("SDL_QUIT event received\n");
                running = 0;
                result = -1;
                break;

            case SDL_MOUSEBUTTONDOWN:
                printf("Mouse button down at (%d, %d)\n", mouse_x, mouse_y);
                // Find which dialog object was clicked
                for (int i = 0; dialog[i].proc != NULL; i++) {
                    // Expand hit detection for bitmap_editor_proc to account for cursor offset
                    int hit_margin_x = (dialog[i].proc == bitmap_editor_proc) ? 10 : 0;
                    int hit_margin_y = (dialog[i].proc == bitmap_editor_proc) ? 10 : 0;

                    if (mouse_x >= dialog[i].x &&
                        mouse_x < dialog[i].x + dialog[i].w + hit_margin_x &&
                        mouse_y >= dialog[i].y &&
                        mouse_y < dialog[i].y + dialog[i].h + hit_margin_y) {

                        printf("Click on dialog object %d\n", i);
                        int ret = dialog[i].proc(MSG_CLICK, &dialog[i], 0);
                        if (ret & D_EXIT) {
                            running = 0;
                            result = i;
                        }
                        if (ret & D_REDRAW) {
                            // Mark for redraw
                            dialog[i].proc(MSG_DRAW, &dialog[i], 0);
                        }
                    }
                }
                break;

            case SDL_KEYDOWN:
                printf("Key pressed: %d\n", event.key.keysym.sym);
                // Convert SDL key to Allegro-style key code
                int key_code = sdl_key_to_allegro(event.key.keysym);

                // Send to focused object or all objects
                int key_handled = 0;
                for (int i = 0; dialog[i].proc != NULL && !key_handled; i++) {
                    int ret = dialog[i].proc(MSG_KEY, &dialog[i], key_code);
                    if (ret & D_EXIT) {
                        running = 0;
                        result = i;
                    }
                    if (ret & D_REDRAW) {
                        dialog[i].proc(MSG_DRAW, &dialog[i], 0);
                    }
                    if (ret & D_USED_CHAR) {
                        key_handled = 1;
                    }
                }

                // Also try MSG_CHAR for character input
                if (!key_handled) {
                    for (int i = 0; dialog[i].proc != NULL && !key_handled; i++) {
                        int ret = dialog[i].proc(MSG_CHAR, &dialog[i], key_code);
                        if (ret & D_EXIT) {
                            running = 0;
                            result = i;
                        }
                        if (ret & D_REDRAW) {
                            dialog[i].proc(MSG_DRAW, &dialog[i], 0);
                        }
                        if (ret & D_USED_CHAR) {
                            key_handled = 1;
                        }
                    }
                }
                break;

            case SDL_MOUSEMOTION:
                // Handle mouse motion for objects that want it
                for (int i = 0; dialog[i].proc != NULL; i++) {
                    int is_inside = (mouse_x >= dialog[i].x &&
                        mouse_x < dialog[i].x + dialog[i].w &&
                        mouse_y >= dialog[i].y &&
                        mouse_y < dialog[i].y + dialog[i].h);

                    // Check if mouse entered or left this object
                    static int last_inside[100] = { 0 }; // Simple tracking

                    if (is_inside && !last_inside[i]) {
                        // Mouse entered
                        dialog[i].proc(MSG_GOTMOUSE, &dialog[i], 0);
                    }
                    else if (!is_inside && last_inside[i]) {
                        // Mouse left
                        dialog[i].proc(MSG_LOSTMOUSE, &dialog[i], 0);
                    }
                    last_inside[i] = is_inside;
                }
                break;
            }
        }

        // Send MSG_IDLE to all objects
        int need_redraw = 0;
        for (int i = 0; dialog[i].proc != NULL; i++) {
            int ret = dialog[i].proc(MSG_IDLE, &dialog[i], 0);
            if (ret & D_EXIT) {
                running = 0;
                result = i;
            }
            if (ret & D_REDRAW) {
                need_redraw = 1;
                dialog[i].proc(MSG_DRAW, &dialog[i], 0);
            }
        }

        // Render at consistent frame rate or when needed
        if (current_time - last_render_time >= min_frame_time || need_redraw) {
            // If we didn't already redraw individual objects, redraw everything
            if (!need_redraw) {
                for (int i = 0; dialog[i].proc != NULL; i++) {
                    dialog[i].proc(MSG_DRAW, &dialog[i], 0);
                }
            }

            SDL_Flip();
            last_render_time = current_time;
        }

        // Small delay to prevent CPU spinning
        SDL_Delay(1);
    }

    // Send MSG_END to all dialog objects
    for (int i = 0; dialog[i].proc != NULL; i++) {
        dialog[i].proc(MSG_END, &dialog[i], 0);
    }

    return result;
}

// Alert dialog function with scaling
int alert(const char* s1, const char* s2, const char* s3,
    const char* b1, const char* b2, int c1, int c2)
{
    DIALOG alert_dialog[8];
    int result;
    int button_count = 0;
    int dialog_width = 0;
    int dialog_height = 0;
    int text_width;
    int i;

    // Calculate dialog dimensions based on text
    if (s1) {
        text_width = strlen(s1) * 8 + 20;
        if (text_width > dialog_width) dialog_width = text_width;
    }
    if (s2) {
        text_width = strlen(s2) * 8 + 20;
        if (text_width > dialog_width) dialog_width = text_width;
    }
    if (s3) {
        text_width = strlen(s3) * 8 + 20;
        if (text_width > dialog_width) dialog_width = text_width;
    }

    // Minimum width
    if (dialog_width < 200) dialog_width = 200;

    // Calculate height based on number of text lines
    dialog_height = 80;  // Base height
    if (s1) dialog_height += 15;
    if (s2) dialog_height += 15;
    if (s3) dialog_height += 15;

    // Shadow box background
    alert_dialog[0].proc = d_shadow_box_proc;
    alert_dialog[0].x = 0;
    alert_dialog[0].y = 0;
    alert_dialog[0].w = dialog_width;  // Store unscaled dimensions
    alert_dialog[0].h = dialog_height;
    alert_dialog[0].fg = 0;
    alert_dialog[0].bg = 0;
    alert_dialog[0].key = 0;
    alert_dialog[0].flags = 0;
    alert_dialog[0].d1 = 0;
    alert_dialog[0].d2 = 0;
    alert_dialog[0].dp = NULL;
    alert_dialog[0].dp2 = NULL;
    alert_dialog[0].dp3 = NULL;

    i = 1;

    // First text line
    if (s1) {
        alert_dialog[i].proc = d_ctext_proc;
        alert_dialog[i].x = dialog_width / 2;  // Unscaled
        alert_dialog[i].y = 15;
        alert_dialog[i].w = 0;
        alert_dialog[i].h = 0;
        alert_dialog[i].fg = GUI_FORE;
        alert_dialog[i].bg = GUI_BACK;
        alert_dialog[i].key = 0;
        alert_dialog[i].flags = 0;
        alert_dialog[i].d1 = 0;
        alert_dialog[i].d2 = 0;
        alert_dialog[i].dp = (void*)s1;
        alert_dialog[i].dp2 = NULL;
        alert_dialog[i].dp3 = NULL;
        i++;
    }

    // Second text line
    if (s2) {
        alert_dialog[i].proc = d_ctext_proc;
        alert_dialog[i].x = (dialog_width) / 2;
        alert_dialog[i].y = (s1 ? 30 : 15);
        alert_dialog[i].w = 0;
        alert_dialog[i].h = 0;
        alert_dialog[i].fg = GUI_FORE;
        alert_dialog[i].bg = GUI_BACK;
        alert_dialog[i].key = 0;
        alert_dialog[i].flags = 0;
        alert_dialog[i].d1 = 0;
        alert_dialog[i].d2 = 0;
        alert_dialog[i].dp = (void*)s2;
        alert_dialog[i].dp2 = NULL;
        alert_dialog[i].dp3 = NULL;
        i++;
    }

    // Third text line
    if (s3) {
        alert_dialog[i].proc = d_ctext_proc;
        alert_dialog[i].x = (dialog_width) / 2;
        alert_dialog[i].y = ((s1 && s2) ? 45 : (s1 || s2) ? 30 : 15);
        alert_dialog[i].w = 0;
        alert_dialog[i].h = 0;
        alert_dialog[i].fg = GUI_FORE;
        alert_dialog[i].bg = GUI_BACK;
        alert_dialog[i].key = 0;
        alert_dialog[i].flags = 0;
        alert_dialog[i].d1 = 0;
        alert_dialog[i].d2 = 0;
        alert_dialog[i].dp = (void*)s3;
        alert_dialog[i].dp2 = NULL;
        alert_dialog[i].dp3 = NULL;
        i++;
    }

    // First button
    if (b1) {
        int button_width = (strlen(b1) + 2) * 8;
        alert_dialog[i].proc = d_button_proc;
        alert_dialog[i].x = ((dialog_width) / 2) - (button_width)-(5);
        alert_dialog[i].y = (dialog_height - 30);
        alert_dialog[i].w = button_width;
        alert_dialog[i].h = 20;
        alert_dialog[i].fg = GUI_FORE;
        alert_dialog[i].bg = GUI_BACK;
        alert_dialog[i].key = c1;
        alert_dialog[i].flags = D_EXIT;
        alert_dialog[i].d1 = 0;
        alert_dialog[i].d2 = 0;
        alert_dialog[i].dp = (void*)b1;
        alert_dialog[i].dp2 = NULL;
        alert_dialog[i].dp3 = NULL;
        button_count++;
        i++;
    }

    // Second button
    if (b2) {
        int button_width = (strlen(b2) + 2) * 8;
        alert_dialog[i].proc = d_button_proc;
        alert_dialog[i].x = ((dialog_width) / 2) + (5);
        alert_dialog[i].y = (dialog_height - 30);
        alert_dialog[i].w = button_width;
        alert_dialog[i].h = 20;
        alert_dialog[i].fg = GUI_FORE;
        alert_dialog[i].bg = GUI_BACK;
        alert_dialog[i].key = c2;
        alert_dialog[i].flags = D_EXIT;
        alert_dialog[i].d1 = 0;
        alert_dialog[i].d2 = 0;
        alert_dialog[i].dp = (void*)b2;
        alert_dialog[i].dp2 = NULL;
        alert_dialog[i].dp3 = NULL;
        button_count++;
        i++;
    }

    // If only one button, center it
    if (button_count == 1 && b1) {
        int button_width = (strlen(b1) + 2) * 8;
        alert_dialog[i - 1].x = ((dialog_width)-(button_width)) / 2;
    }

    // Terminator
    alert_dialog[i].proc = NULL;
    alert_dialog[i].x = 0;
    alert_dialog[i].y = 0;
    alert_dialog[i].w = 0;
    alert_dialog[i].h = 0;
    alert_dialog[i].fg = 0;
    alert_dialog[i].bg = 0;
    alert_dialog[i].key = 0;
    alert_dialog[i].flags = 0;
    alert_dialog[i].d1 = 0;
    alert_dialog[i].d2 = 0;
    alert_dialog[i].dp = NULL;
    alert_dialog[i].dp2 = NULL;
    alert_dialog[i].dp3 = NULL;

    // Center the dialog on screen
    position_dialog(alert_dialog, (SCREEN_W - dialog_width) / 2,
        (SCREEN_H - dialog_height) / 2);

    // Set dialog colors
    set_dialog_color(alert_dialog, GUI_FORE, GUI_BACK);

    // Run the dialog
    result = do_dialog(alert_dialog, -1);

    // Figure out which button was pressed
    // The result is the index of the dialog element that caused the exit
    if (b1 && b2) {
        // Find which button index was pressed
        int first_button_index = 1;
        if (s1) first_button_index++;
        if (s2) first_button_index++;
        if (s3) first_button_index++;

        if (result == first_button_index) return 1;  // First button
        if (result == first_button_index + 1) return 2;  // Second button
    }

    return 1;  // Default to first button
}

void allegro_init(void) {
    printf("Initializing SDL...\n");

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    // Create window at your desired display resolution (1024x768)
    sdl_window = SDL_CreateWindow(
        "Turaco Sprite Editor",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        gfx_hres,  // 1024
        gfx_vres,  // 768
        SDL_WINDOW_SHOWN
    );

    if (!sdl_window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    // Create renderer
    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
    if (!sdl_renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        exit(1);
    }

    // This tells SDL to automatically scale 320x240 content to fill the 1024x768 window
    if (SDL_RenderSetLogicalSize(sdl_renderer, 320, 240) != 0) {
        printf("SDL_RenderSetLogicalSize failed: %s\n", SDL_GetError());
        // Continue anyway - it's not fatal
    }
    else {
        printf("SDL logical size set to 320x240 (automatic scaling to %dx%d)\n", gfx_hres, gfx_vres);
    }

    // Create texture at the ORIGINAL resolution (320x240)
    sdl_texture = SDL_CreateTexture(
        sdl_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        320,  // Original resolution width
        240   // Original resolution height
    );

    if (!sdl_texture) {
        printf("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(sdl_renderer);
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        exit(1);
    }

    // Allocate pixel buffer for ORIGINAL resolution (320x240)
    sdl_pixels = malloc(320 * 240 * sizeof(Uint32));
    if (!sdl_pixels) {
        printf("Could not allocate pixel buffer!\n");
        SDL_DestroyTexture(sdl_texture);
        SDL_DestroyRenderer(sdl_renderer);
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        exit(1);
    }

    // Clear pixel buffer
    memset(sdl_pixels, 0, 320 * 240 * sizeof(Uint32));

    // Now create the screen bitmap that points to our SDL pixel buffer
    screen = create_bitmap(320, 240);  // Original resolution!
    if (!screen) {
        printf("Failed to create screen bitmap!\n");
        exit(1);
    }

    printf("SDL initialized successfully\n");
    printf("Window: %dx%d, Logical: 320x240 (automatic scaling)\n", gfx_hres, gfx_vres);
}



int main(int argc, char** argv)
{
    int ret;
    ret = parse_command_line(argc, argv);

    allegro_init();

    // Initialize subsystems
    install_keyboard();
    install_mouse();
    Init_Subsystems();
    setup_palette();
    //  init_main_dialog_scaling();

    printf("=== Turaco SDL2 Port ===\n");


    if (ret == RET_DOIT)
    {
        REGULAR_CURSOR();
        printf("3. Starting main dialog...\n");

        // The main dialog should have its own render loop
        do_dialog(main_dialog, -1);

        DeInit_Subsystems();
        allegro_exit();
        end_blurb();
    }
    else {
        allegro_exit();
        if (ret < RET_USAGE)
            end_blurb();

        if (ret == RET_USAGE)    usage();
        if (ret == RET_VERSION)  version_info();
        if (ret == RET_LIST)     list_games();
        if (ret == RET_LISTFULL) list_full();
        if (ret == RET_LISTHTML) list_html();
        if (ret == RET_LISTROMS) list_roms();
    }

    return 0;
}

/*
void foo(void)
{
    // dump out the font to a file.
    int fpos;
    FILE * fp;
    fp = fopen("font.bin", "w");
    if (fp)
    {
    fwrite (font->dat.dat_8x8, 1, 224*8, fp);
    fclose(fp);
    }
}
*/

