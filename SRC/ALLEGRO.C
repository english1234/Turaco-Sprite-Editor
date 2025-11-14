#include "../INCLUDE/ALLEGRO.H"
#include SDL_PATH
#include <stdio.h>
#include <windows.h>
#include "../INCLUDE/GENERAL.H"
#include "../INCLUDE/GUIPAL.H"
#include "../INCLUDE/bitmaped.h"
#include "../INCLUDE/UTIL.H"

int text_background_mode = -1;
int palette_dirty = 1;
Uint32 palette_rgb[256];  // RGB lookup table for performance

extern MYBITMAP* screen;
extern FONT* font;
extern FONT_8x8  my_new_font_data[];
extern PALETTE desktop_palette;
extern volatile int mouse_x;
extern volatile int mouse_y;
extern volatile int mouse_z;
extern volatile int mouse_b;

extern SDL_Window* sdl_window;
extern SDL_Renderer* sdl_renderer;
extern SDL_Texture* sdl_texture;
extern Uint32* sdl_pixels;

#ifndef strdup
#define strdup _strdup
#endif

// Text procedure - no scaling
int d_text_proc(int msg, DIALOG* d, int c) {
    if (msg == MSG_DRAW && d->dp) {
        // Use original coordinates - no scaling
        int x = d->x;
        int y = d->y;
        int w = d->w;
        int h = d->h;

        // Fill background if needed
        if (text_background_mode >= 0) {
            rectfill(screen, x, y, x + w - 1, y + h - 1, d->bg);
        }

        // Draw text
        textout(screen, font, (char*)d->dp, x, y, d->fg);
    }
    return D_O_K;
}

// Clear the screen/dialog area - no scaling
int d_clear_proc(int msg, DIALOG* d, int c) {
    if (msg == MSG_DRAW) {
        // Use GUI_BACK if d->bg is 0 (black)
        int bg_color = (d->bg == 0) ? GUI_BACK : d->bg;
        clear_to_color(screen, bg_color);
        clear_to_color(screen, d->bg);
    }
    return D_O_K;
}

// Draw shadowed box - no scaling
int d_shadow_box_proc(int msg, DIALOG* d, int c) {
    if (msg == MSG_DRAW) {
        // Use original coordinates - no scaling
        int x = d->x;
        int y = d->y;
        int w = d->w;
        int h = d->h;

        // Draw main box
        rectfill(screen, x, y, x + w - 1, y + h - 1, d->bg);

        // Draw 3D border
        rect(screen, x, y, x + w - 1, y + h - 1, d->fg);

        // Draw shadow effect
        line(screen, x + 1, y + h, x + w, y + h, 8);
        line(screen, x + w, y + 1, x + w, y + h, 8);
    }
    return D_O_K;
}

// Draw centered text - no scaling
int d_ctext_proc(int msg, DIALOG* d, int c) {
    if (msg == MSG_DRAW && d->dp) {
        // Use original coordinates - no scaling
        int x = d->x;
        int y = d->y;
        int w = d->w;
        int h = d->h;

        char* text = (char*)d->dp;
        int text_width = strlen(text) * 8;  // Original character width
        int text_height = 8;                // Original character height
        int center_x = x + (w - text_width) / 2;
        int center_y = y + (h - text_height) / 2;
        textout(screen, font, text, center_x, center_y, d->fg);
    }
    return D_O_K;
}

// Improved d_textbox_proc with word wrap
int d_textbox_proc(int msg, DIALOG* d, int c) {
    static int scroll_pos = 0;
    static char** wrapped_lines = NULL;
    static int wrapped_line_count = 0;
    static int last_width = 0;

    char* text = (char*)d->dp;
    if (!text) return D_O_K;

    // Calculate text metrics
    int char_width = 8;
    int char_height = 8;
    int chars_per_line = (d->w - 16) / char_width;  // Leave room for scrollbar
    int lines_visible = d->h / char_height;

    // Re-wrap text if width changed or first time
    if (wrapped_lines == NULL || last_width != d->w) {
        // Free old wrapped lines
        if (wrapped_lines) {
            for (int i = 0; i < wrapped_line_count; i++) {
                free(wrapped_lines[i]);
            }
            free(wrapped_lines);
        }

        // Count how many wrapped lines we'll need (estimate)
        int estimated_lines = strlen(text) / chars_per_line + 100;
        wrapped_lines = (char**)malloc(estimated_lines * sizeof(char*));
        wrapped_line_count = 0;

        // Wrap the text
        char* src = text;
        while (*src && wrapped_line_count < estimated_lines - 1) {
            // Skip leading whitespace on new lines
            while (*src == ' ' || *src == '\t') src++;
            if (!*src) break;

            // Find the end of this wrapped line
            char* line_end = src;
            char* last_space = NULL;
            int chars_on_line = 0;

            while (*line_end && *line_end != '\n' && chars_on_line < chars_per_line) {
                if (*line_end == ' ' || *line_end == '\t') {
                    last_space = line_end;
                }
                line_end++;
                chars_on_line++;
            }

            // If we hit a newline or end of string, use that
            if (*line_end == '\n' || *line_end == '\0') {
                int len = line_end - src;
                wrapped_lines[wrapped_line_count] = (char*)malloc(len + 1);
                memcpy(wrapped_lines[wrapped_line_count], src, len);
                wrapped_lines[wrapped_line_count][len] = '\0';
                wrapped_line_count++;
                src = (*line_end == '\n') ? line_end + 1 : line_end;
            }
            // Otherwise, break at last space if possible
            else if (last_space && last_space > src) {
                int len = last_space - src;
                wrapped_lines[wrapped_line_count] = (char*)malloc(len + 1);
                memcpy(wrapped_lines[wrapped_line_count], src, len);
                wrapped_lines[wrapped_line_count][len] = '\0';
                wrapped_line_count++;
                src = last_space + 1;
            }
            // No space found, hard break
            else {
                int len = line_end - src;
                wrapped_lines[wrapped_line_count] = (char*)malloc(len + 1);
                memcpy(wrapped_lines[wrapped_line_count], src, len);
                wrapped_lines[wrapped_line_count][len] = '\0';
                wrapped_line_count++;
                src = line_end;
            }
        }

        last_width = d->w;
    }

    int total_lines = wrapped_line_count;

    switch (msg) {
    case MSG_START:
        scroll_pos = d->d2;  // Use d2 for initial scroll position
        if (scroll_pos < 0) scroll_pos = 0;
        break;

    case MSG_DRAW:
        // Draw text box background and border
        rectfill(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, d->bg);
        rect(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, d->fg);

        // Draw scrollbar if needed
        if (total_lines > lines_visible) {
            int scrollbar_width = 12;
            int scrollbar_x = d->x + d->w - scrollbar_width;
            int scrollbar_height = d->h - 2;
            int thumb_height = (lines_visible * scrollbar_height) / total_lines;
            if (thumb_height < 10) thumb_height = 10;
            int max_scroll = total_lines - lines_visible;
            int thumb_y = d->y + 1 + (scroll_pos * (scrollbar_height - thumb_height)) / max_scroll;

            // Draw scrollbar track
            rectfill(screen, scrollbar_x, d->y + 1, d->x + d->w - 2, d->y + d->h - 2, 8);

            // Draw scroll thumb
            rectfill(screen, scrollbar_x + 1, thumb_y, d->x + d->w - 3,
                thumb_y + thumb_height - 1, 7);
            rect(screen, scrollbar_x + 1, thumb_y, d->x + d->w - 3,
                thumb_y + thumb_height - 1, 15);
        }

        // Draw visible text lines
        for (int i = 0; i < lines_visible && (scroll_pos + i) < total_lines; i++) {
            int line_idx = scroll_pos + i;
            int line_y = d->y + 2 + (i * char_height);
            textout(screen, font, wrapped_lines[line_idx], d->x + 2, line_y, d->fg);
        }
        break;

    case MSG_WHEEL:
        // Mouse wheel scrolling
        scroll_pos -= c * 3; // Scroll 3 lines per wheel notch
        if (scroll_pos < 0) scroll_pos = 0;
        if (scroll_pos > total_lines - lines_visible)
            scroll_pos = total_lines - lines_visible;
        if (scroll_pos < 0) scroll_pos = 0;
        return D_REDRAWME;

    case MSG_KEY:
        if (c == (SDL_SCANCODE_UP << 8)) {
            if (scroll_pos > 0) {
                scroll_pos--;
                return D_REDRAWME;
            }
        }
        else if (c == (SDL_SCANCODE_DOWN << 8)) {
            if (scroll_pos < total_lines - lines_visible) {
                scroll_pos++;
                return D_REDRAWME;
            }
        }
        else if (c == (SDL_SCANCODE_PAGEUP << 8)) {
            scroll_pos -= lines_visible;
            if (scroll_pos < 0) scroll_pos = 0;
            return D_REDRAWME;
        }
        else if (c == (SDL_SCANCODE_PAGEDOWN << 8)) {
            scroll_pos += lines_visible;
            if (scroll_pos > total_lines - lines_visible)
                scroll_pos = total_lines - lines_visible;
            if (scroll_pos < 0) scroll_pos = 0;
            return D_REDRAWME;
        }
        else if (c == (SDL_SCANCODE_HOME << 8)) {
            scroll_pos = 0;
            return D_REDRAWME;
        }
        else if (c == (SDL_SCANCODE_END << 8)) {
            scroll_pos = total_lines - lines_visible;
            if (scroll_pos < 0) scroll_pos = 0;
            return D_REDRAWME;
        }
        break;

    case MSG_CLICK:
        // Handle click on scrollbar
        if (total_lines > lines_visible &&
            mouse_x >= d->x + d->w - 12 && mouse_x < d->x + d->w) {
            // Clicked on scrollbar - set scroll position based on click
            int click_rel_y = mouse_y - d->y;
            scroll_pos = (click_rel_y * total_lines) / d->h;
            if (scroll_pos < 0) scroll_pos = 0;
            if (scroll_pos > total_lines - lines_visible)
                scroll_pos = total_lines - lines_visible;
            return D_REDRAWME;
        }
        break;

    case MSG_END:
        // Clean up wrapped lines when dialog closes
        if (wrapped_lines) {
            for (int i = 0; i < wrapped_line_count; i++) {
                free(wrapped_lines[i]);
            }
            free(wrapped_lines);
            wrapped_lines = NULL;
            wrapped_line_count = 0;
            last_width = 0;
        }
        break;
    }

    return D_O_K;
}

int d_button_proc(int msg, DIALOG* d, int c) {

    switch (msg) {
    case MSG_DRAW:
        // Draw at original coordinates - no scaling needed!
        rectfill(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, d->bg);
        rect(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, d->fg);

        if (d->dp) {
            char* text = (char*)d->dp;

            // Parse text to remove '&' character
            char display_text[256];
            int display_pos = 0;
            int underline_pos = -1;

            for (int j = 0; text[j] != '\0' && display_pos < 255; j++) {
                if (text[j] == '&') {
                    underline_pos = display_pos;  // Mark position for underline
                    // Skip the '&' - don't add to display_text
                }
                else {
                    display_text[display_pos++] = text[j];
                }
            }
            display_text[display_pos] = '\0';

            int text_width = display_pos * 8;  // Use display length, not original
            int text_height = 8;
            int text_x = d->x + (d->w - text_width) / 2;
            int text_y = d->y + (d->h - text_height) / 2;
            textout(screen, font, display_text, text_x, text_y, d->fg);

            // Draw underline under the shortcut key
            if (underline_pos >= 0) {
                int underline_x = text_x + (underline_pos * 8);
                int underline_y = text_y + 7;
                hline(screen, underline_x, underline_y, underline_x + 7, d->fg);
            }
        }
        break;

    case MSG_CLICK:
        if (mouse_x >= d->x && mouse_x < d->x + d->w &&
            mouse_y >= d->y && mouse_y < d->y + d->h) {
            if (d->flags & D_EXIT) {
                return D_CLOSE;
            }
        }
        break;
    }
    return D_O_K;
}

// Draw a horizontal line
void hline(MYBITMAP* bmp, int x1, int y, int x2, int color) {
    if (!bmp) return;

    // Ensure x1 <= x2
    if (x1 > x2) {
        int temp = x1;
        x1 = x2;
        x2 = temp;
    }

    // Clip to bitmap bounds
    if (y < 0 || y >= bmp->h) return;
    if (x2 < 0 || x1 >= bmp->w) return;
    if (x1 < 0) x1 = 0;
    if (x2 >= bmp->w) x2 = bmp->w - 1;

    // Draw the horizontal line
    for (int x = x1; x <= x2; x++) {
        putpixel(bmp, x, y, color);
    }
}

// Draw a vertical line
void vline(MYBITMAP* bmp, int x, int y1, int y2, int color) {
    if (!bmp) return;

    // Ensure y1 <= y2
    if (y1 > y2) {
        int temp = y1;
        y1 = y2;
        y2 = temp;
    }

    // Clip to bitmap bounds
    if (x < 0 || x >= bmp->w) return;
    if (y2 < 0 || y1 >= bmp->h) return;
    if (y1 < 0) y1 = 0;
    if (y2 >= bmp->h) y2 = bmp->h - 1;

    // Draw the vertical line
    for (int y = y1; y <= y2; y++) {
        putpixel(bmp, x, y, color);
    }
}

// Set text mode to ensure text is visible
void setup_visible_text(void) {
    // Use solid background for text to ensure visibility
    text_mode(0); // 0 = black background for text
}

int putpixel(MYBITMAP* bmp, int x, int y, int color) {
    if (!bmp || x < 0 || x >= bmp->w || y < 0 || y >= bmp->h) {
        return 0; // Failed
    }

    if (bmp->dat) {
        unsigned char* pixels = (unsigned char*)bmp->dat;
        pixels[y * bmp->w + x] = color;
        return 1; // Success
    }

    if (bmp->line && bmp->line[y]) {
        unsigned char* line = (unsigned char*)bmp->line[y];
        line[x] = color;
        return 1; // Success
    }

    return 0; // Failed
}

int getpixel(MYBITMAP* bmp, int x, int y) {
    if (!bmp || x < 0 || x >= bmp->w || y < 0 || y >= bmp->h) {
        return -1;
    }

    if (bmp->dat) {
        unsigned char* pixels = (unsigned char*)bmp->dat;
        return pixels[y * bmp->w + x];
    }

    if (bmp->line && bmp->line[y]) {
        unsigned char* line = (unsigned char*)bmp->line[y];
        return line[x];
    }

    return -1;
}

// Mouse visibility
void show_mouse(MYBITMAP* bmp) {
    // bmp=NULL hides mouse, bmp=screen shows mouse
    if (bmp == NULL) {
        SDL_ShowCursor(SDL_DISABLE);
    }
    else {
        SDL_ShowCursor(SDL_ENABLE);
    }
}

// Text mode (transparent/solid background)
void text_mode(int mode) {
    // -1 = transparent, >=0 = solid with that color
    // You'll need to track this in a global variable for text rendering

    text_background_mode = mode;
}

void textout(MYBITMAP* bmp, FONT* fnt, const char* text, int x, int y, int color) {
    if (!bmp || !text || !fnt || !fnt->data) return;

    FONT_8x8* font_data = (FONT_8x8*)fnt->data;
    int len = strlen(text);

    // Draw background if needed
    if (text_background_mode >= 0) {
        int text_width = len * 8;
        int text_height = 8;
        rectfill(bmp, x, y, x + text_width - 1, y + text_height - 1, text_background_mode);
    }

    // Draw each character at original size
    for (int i = 0; i < len; i++) {
        unsigned char c = text[i];
        if (c < 32 || c > 126) continue;

        unsigned char* char_data = font_data->dat[c - 0x20];

        for (int row = 0; row < 8; row++) {
            unsigned char pattern = char_data[row];
            for (int col = 0; col < 8; col++) {
                if (pattern & (1 << (7 - col))) {
                    putpixel(bmp, x + i * 8 + col, y + row, color);
                }
            }
        }
    }
}

// Regular centered text (uses consistent 2x scaling)
void textout_centre(MYBITMAP* bmp, FONT* font, const char* text, int x, int y, int color) {
    int text_width = strlen(text) * 8;
    int centered_x = x - (text_width / 2);
    textout(bmp, font, text, centered_x, y, color);
}

// Stretched blit
void stretch_blit(MYBITMAP* src, MYBITMAP* dst,
    int src_x, int src_y, int src_w, int src_h,
    int dst_x, int dst_y, int dst_w, int dst_h) {

    if (!src || !dst) return;

    float scale_x = (float)dst_w / src_w;
    float scale_y = (float)dst_h / src_h;

    for (int dy = 0; dy < dst_h; dy++) {
        for (int dx = 0; dx < dst_w; dx++) {
            int sx = src_x + (int)(dx / scale_x);
            int sy = src_y + (int)(dy / scale_y);

            // Clamp source coordinates
            if (sx < 0) sx = 0;
            if (sx >= src->w) sx = src->w - 1;
            if (sy < 0) sy = 0;
            if (sy >= src->h) sy = src->h - 1;

            int color = getpixel(src, sx, sy);
            if (color >= 0) { // Valid pixel
                putpixel(dst, dst_x + dx, dst_y + dy, color);
            }
        }
    }
}

// Dialog message broadcasting
void broadcast_dialog_message(int message, int param) {
    // This would send a message to all dialogs in your system
    // For now, implement as empty or add to your dialog system
}

int dialog_message(int msg, DIALOG* d, int c) {
    if (!d || !d->proc) return D_O_K;
    return d->proc(msg, d, c);
}

void object_message(DIALOG* d, int msg, int c) {
    if (d && d->proc) {
        d->proc(msg, d, c);
    }
}

#ifndef SEND_MESSAGE
#define SEND_MESSAGE(dlg, msg, c) (dlg)->proc((msg), (dlg), (c))
#endif

void allegro_exit(void) {
    if (screen) {
        destroy_bitmap(screen);
        screen = NULL;
    }

    if (sdl_texture) {
        SDL_DestroyTexture(sdl_texture);
        sdl_texture = NULL;
    }

    if (sdl_renderer) {
        SDL_DestroyRenderer(sdl_renderer);
        sdl_renderer = NULL;
    }

    if (sdl_window) {
        SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
    }

    // Don't free sdl_pixels here - it's owned by the screen bitmap which will free it

    SDL_Quit();
}

void install_keyboard(void) {
    // SDL keyboard is automatically initialized
}

void install_mouse(void) {
    // SDL mouse is automatically initialized  
    SDL_ShowCursor(SDL_ENABLE);
}

void install_timer(void) {
    // SDL timer is automatically initialized
}

void install_int(void (*handler)(void), int interval) {
    // SDL timers work differently - you might use SDL_AddTimer
}

// Graphics mode
int set_gfx_mode(int card, int w, int h, int v_w, int v_h) {
    // For now, just return success - actual mode set in SDL initialization
    return 0;
}

void set_palette(PALETTE pal) {
    memcpy(desktop_palette, pal, sizeof(PALETTE));
    palette_dirty = 1;  // Trigger palette recalculation
}

void set_color(int index, RGB* color) {
    if (index >= 0 && index < 256) {
        desktop_palette[index] = *color;
        palette_dirty = 1;
    }
}

// Force palette to be rebuilt on next SDL_Flip
void mark_palette_dirty(void) {
    palette_dirty = 1;
}

void set_palette_range(PALETTE pal, int from, int to, int retracesync) {
    for (int i = from; i <= to && i < 256; i++) {
        desktop_palette[i] = pal[i];
    }
    palette_dirty = 1;  // ← MARK PALETTE DIRTY
}

// Drawing functions
void clear(MYBITMAP* bmp) {
    if (!bmp) return;
    clear_to_color(bmp, 0);  // Clear to black
}

void line(MYBITMAP* bmp, int x1, int y1, int x2, int y2, int color) {
    if (!bmp) return;

    // Bresenham's line algorithm
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        putpixel(bmp, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void rect(MYBITMAP* bmp, int x1, int y1, int x2, int y2, int color) {
    if (!bmp) return;

    line(bmp, x1, y1, x2, y1, color); // Top
    line(bmp, x2, y1, x2, y2, color); // Right
    line(bmp, x2, y2, x1, y2, color); // Bottom
    line(bmp, x1, y2, x1, y1, color); // Left
}

void floodfill(MYBITMAP* bmp, int x, int y, int color) {
    // Simple recursive flood fill (be careful with large areas)
    if (!bmp || x < 0 || x >= bmp->w || y < 0 || y >= bmp->h) return;

    int old_color = getpixel(bmp, x, y);
    if (old_color == color) return;

    // Stack-based would be better, but recursive is simpler
    putpixel(bmp, x, y, color);

    if (x > 0 && getpixel(bmp, x - 1, y) == old_color)
        floodfill(bmp, x - 1, y, color);
    if (x < bmp->w - 1 && getpixel(bmp, x + 1, y) == old_color)
        floodfill(bmp, x + 1, y, color);
    if (y > 0 && getpixel(bmp, x, y - 1) == old_color)
        floodfill(bmp, x, y - 1, color);
    if (y < bmp->h - 1 && getpixel(bmp, x, y + 1) == old_color)
        floodfill(bmp, x, y + 1, color);
}

// Sprite functions
// Generic sprite flip function
void draw_sprite_ex(MYBITMAP* bmp, MYBITMAP* sprite, int x, int y, int flip_mode) {
    if (!bmp || !sprite) return;

    for (int sy = 0; sy < sprite->h; sy++) {
        for (int sx = 0; sx < sprite->w; sx++) {
            int dx = x, dy = y;

            switch (flip_mode) {
            case FLIP_H:
                dx += sprite->w - 1 - sx;
                dy += sy;
                break;
            case FLIP_V:
                dx += sx;
                dy += sprite->h - 1 - sy;
                break;
            case FLIP_VH:
                dx += sprite->w - 1 - sx;
                dy += sprite->h - 1 - sy;
                break;
            default: // Normal
                dx += sx;
                dy += sy;
            }

            if (dx >= 0 && dx < bmp->w && dy >= 0 && dy < bmp->h) {
                putpixel(bmp, dx, dy, getpixel(sprite, sx, sy));
            }
        }
    }
}


void rotate_sprite(MYBITMAP* bmp, MYBITMAP* sprite, int x, int y, fixed angle) {
    // Simple rotation - you might want a more sophisticated implementation
    // This is a placeholder that just draws the sprite normally
    blit(sprite, bmp, 0, 0, x, y, sprite->w, sprite->h);
}

// Dialog functions
void centre_dialog(DIALOG* dialog) {
    if (!dialog) return;

    // Calculate dialog width and height
    int w = 0, h = 0;
    for (int i = 0; dialog[i].proc != NULL; i++) {
        int right = dialog[i].x + dialog[i].w;
        int bottom = dialog[i].y + dialog[i].h;
        if (right > w) w = right;
        if (bottom > h) h = bottom;
    }

    // Center on screen
    int x = (SCREEN_W - w) / 2;
    int y = (SCREEN_H - h) / 2;

    position_dialog(dialog, x, y);
}

void set_dialog_color(DIALOG* dialog, int fg, int bg) {
    if (!dialog) return;

    for (int i = 0; dialog[i].proc != NULL; i++) {
        dialog[i].fg = fg;
        dialog[i].bg = bg;
    }
}

int do_menu(MENU* menu, int x, int y) {
    if (!menu) return -1;

    // Calculate menu dimensions
    int menu_width = 0;
    int menu_height = 0;
    int item_count = 0;

    // First pass: calculate menu size
    MENU* current = menu;
    while (current->text != NULL) {
        // Remove '&' character when calculating width
        int text_width = 0;
        for (const char* p = current->text; *p; p++) {
            if (*p != '&') text_width++;
        }
        text_width *= 8; // 8 pixels per character

        if (text_width > menu_width) menu_width = text_width;

        // Check for separator (empty string)
        if (strlen(current->text) == 0) {
            menu_height += 5; // Separator is shorter
        }
        else {
            menu_height += 16; // Regular item height (increased from 10)
        }
        item_count++;
        current++;
    }

    // Add padding
    menu_width += 20; // More padding for readability
    menu_height += 8;

    // Ensure menu stays on screen
    if (x + menu_width > SCREEN_W) x = SCREEN_W - menu_width;
    if (y + menu_height > SCREEN_H) y = SCREEN_H - menu_height;
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    // Save area under menu
    MYBITMAP* saved_area = create_bitmap(menu_width, menu_height);
    if (saved_area) {
        blit(screen, saved_area, x, y, 0, 0, menu_width, menu_height);
    }

    // Menu interaction loop
    int selected_index = -1;
    int menu_active = 1;
    int last_highlighted = -1;

    while (menu_active) {
        // Redraw menu background and border EVERY FRAME
        rectfill(screen, x, y, x + menu_width - 1, y + menu_height - 1, GUI_BACK);
        rect(screen, x, y, x + menu_width - 1, y + menu_height - 1, GUI_FORE);

        // Draw shadow for depth
        hline(screen, x + 2, y + menu_height, x + menu_width, 8);
        vline(screen, x + menu_width, y + 2, y + menu_height, 8);

        // Draw menu items
        current = menu;
        int item_y = y + 4;
        int current_highlighted = -1;

        for (int i = 0; current->text != NULL; i++) {
            // Check for separator (empty string)
            if (strlen(current->text) == 0) {
                // Draw separator line
                int sep_y = item_y + 2;
                hline(screen, x + 5, sep_y, x + menu_width - 5, GUI_MID);
                item_y += 5;
                current++;
                continue;
            }

            // Check if mouse is over this item
            if (mouse_x >= x && mouse_x < x + menu_width &&
                mouse_y >= item_y && mouse_y < item_y + 16) {
                if (!(current->flags & D_DISABLED)) {
                    current_highlighted = i;
                }
            }

            // Highlight background if mouse is over this item
            if (current_highlighted == i) {
                rectfill(screen, x + 2, item_y, x + menu_width - 3, item_y + 15, 1); // Blue highlight
            }

            // Determine text color
            int text_color = (current->flags & D_DISABLED) ? 8 : 15; // Gray if disabled, white if enabled
            if (current_highlighted == i) {
                text_color = 15; // White for highlighted
            }

            // Draw checkmark if selected
            if (current->flags & D_SELECTED) {
                textout(screen, font, "v", x + 4, item_y + 4, text_color); // Use 'v' instead of unicode
            }

            // Parse menu text (remove '&')
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

            // Draw menu text
            int text_x = x + 20;
            textout(screen, font, display_text, text_x, item_y + 4, text_color);

            // Draw underline for keyboard shortcut
            if (underline_pos >= 0 && current_highlighted != i) {
                int underline_x = text_x + (underline_pos * 8);
                hline(screen, underline_x, item_y + 12, underline_x + 7, text_color);
            }

            // Draw submenu arrow if exists
            if (current->child != NULL) {
                textout(screen, font, ">", x + menu_width - 16, item_y + 4, text_color); // Use '>' instead of unicode
            }

            item_y += 16;
            current++;
        }

        // Force immediate screen update
        SDL_Flip();

        // Process events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            update_mouse_state(&event);

            switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    // Check if click is on a menu item
                    current = menu;
                    item_y = y + 4;
                    for (int i = 0; current->text != NULL; i++) {
                        // Skip separators
                        if (strlen(current->text) == 0) {
                            item_y += 5;
                            current++;
                            continue;
                        }

                        if (mouse_x >= x && mouse_x < x + menu_width &&
                            mouse_y >= item_y && mouse_y < item_y + 16) {
                            if (!(current->flags & D_DISABLED)) {
                                selected_index = i;
                                menu_active = 0;
                            }
                            break;
                        }
                        item_y += 16;
                        current++;
                    }

                    // If click outside menu, cancel
                    if (mouse_x < x || mouse_x >= x + menu_width ||
                        mouse_y < y || mouse_y >= y + menu_height) {
                        selected_index = -1;
                        menu_active = 0;
                    }
                }
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    selected_index = -1;
                    menu_active = 0;
                }
                break;

            case SDL_QUIT:
                selected_index = -1;
                menu_active = 0;
                break;
            }
        }

        SDL_Delay(10); // Prevent busy waiting
    }

    // Restore area under menu
    if (saved_area) {
        blit(saved_area, screen, 0, 0, x, y, menu_width, menu_height);
        destroy_bitmap(saved_area);
    }

    // Force screen update to show restoration
    SDL_Flip();

    return selected_index;
}

int d_edit_proc(int msg, DIALOG* d, int c) {
    static char* edit_buffer = NULL;
    static int cursor_pos = 0;
    static int cursor_blink = 0;
    static Uint32 last_blink_time = 0;

    switch (msg) {
    case MSG_START:
        // Initialize edit buffer
        if (d->dp && !edit_buffer) {
            edit_buffer = (char*)d->dp;
            cursor_pos = strlen(edit_buffer);
        }
        break;

    case MSG_DRAW:
        // Draw edit box background and border
        rectfill(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, d->bg);
        rect(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, d->fg);

        if (edit_buffer) {
            // Draw text
            textout(screen, font, edit_buffer, d->x + 2, d->y + 2, d->fg);

            // Draw blinking cursor
            Uint32 current_time = SDL_GetTicks();
            if (current_time - last_blink_time > 500) { // Blink every 500ms
                cursor_blink = !cursor_blink;
                last_blink_time = current_time;
            }

            if (cursor_blink && (d->flags & D_GOTFOCUS)) {
                int cursor_x = d->x + 2 + (cursor_pos * 8);
                line(screen, cursor_x, d->y + 1, cursor_x, d->y + d->h - 2, d->fg);
            }
        }
        break;

    case MSG_KEY:
        if (edit_buffer && (d->flags & D_GOTFOCUS)) {
            if (c == SDL_SCANCODE_BACKSPACE << 8) {
                // Backspace
                if (cursor_pos > 0) {
                    cursor_pos--;
                    edit_buffer[cursor_pos] = '\0';
                    return D_REDRAWME;
                }
            }
            else if (c == SDL_SCANCODE_DELETE << 8) {
                // Delete - clear the buffer
                edit_buffer[0] = '\0';
                cursor_pos = 0;
                return D_REDRAWME;
            }
            else if (c == SDL_SCANCODE_RETURN << 8) {
                // Enter - exit if this is an exit field
                if (d->flags & D_EXIT) {
                    return D_CLOSE;
                }
            }
            else {
                // Regular character input
                char ch = (c >> 8) & 0xFF;
                if (ch >= 32 && ch <= 126) { // Printable ASCII
                    if (cursor_pos < 255) { // Buffer limit
                        edit_buffer[cursor_pos] = ch;
                        cursor_pos++;
                        edit_buffer[cursor_pos] = '\0';
                        return D_REDRAWME;
                    }
                }
            }
        }
        break;

    case MSG_CLICK:
        // Gain focus when clicked
        return D_WANTFOCUS;

    case MSG_GOTFOCUS:
    case MSG_LOSTFOCUS:
        cursor_blink = 1; // Reset cursor blink
        last_blink_time = SDL_GetTicks();
        return D_REDRAWME;
    }

    return D_O_K;
}

/* Helper function to safely get item text from either getter or array */
static const char* d_list_get_item_text(void* dp, int use_getter, char** items, int index, int item_count) {
    if (use_getter) {
        char* (*getter)(int, int*) = (char* (*)(int, int*))dp;
        __try {
            char* result = getter(index, NULL);
            return result ? result : "(error)";
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return "(error)";
        }
    }
    else {
        if (index >= 0 && index < item_count && items && items[index]) {
            return items[index];
        }
        return "(null)";
    }
}

int d_list_proc(int msg, DIALOG* d, int c) {
    static int scroll_pos = 0;
    static int selected_item = -1;

    char** items = NULL;
    char* (*getter)(int, int*) = NULL;
    int item_count = 0;
    int use_getter = 0;
    uintptr_t dp_addr;

    if (!d || !d->dp) return D_O_K;

    /* Allegro list boxes support two modes:
     * 1. dp points to char** array (ends with NULL)
     * 2. dp points to a getter function: char* (*getter)(int index, int *list_size)
     */

    dp_addr = (uintptr_t)d->dp;
    getter = (char* (*)(int, int*))d->dp;

    /* Try to call as getter to get list size */
    __try {
        int size = 0;
        char* result = getter(-1, &size);

        /* Valid getter should return NULL and set size to positive value */
        if (result == NULL && size > 0 && size < 10000) {
            /* Success! This is a getter function */
            use_getter = 1;
            item_count = size;
        }
        else if (size > 0 && size < 10000) {
            /* Some getters might not return NULL but still set size correctly */
            use_getter = 1;
            item_count = size;
        }
        else {
            /* Failed getter test, try as array */
            use_getter = 0;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        /* Exception occurred, definitely not a valid function */
        use_getter = 0;
    }

    /* If getter detection failed, try as array */
    if (!use_getter) {
        /* Check if it's a valid pointer for an array */
        if (dp_addr > 0x10000 && (dp_addr & 0x3) == 0) {
            items = (char**)d->dp;

            /* Count items in array */
            __try {
                while (items[item_count] != NULL && item_count < 1000) {
                    item_count++;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                item_count = 0;
            }
        }
        else {
            /* Invalid pointer - can't display list */
            if (msg == MSG_DRAW) {
                rectfill(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, d->bg);
                rect(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, d->fg);
                textout(screen, font, "(no items)", d->x + 10, d->y + 10, d->fg);
            }
            return D_O_K;
        }
    }

    /* Calculate visible items */
    int visible_items = d->h / 10; /* 10 pixels per item */
    int i;

    switch (msg) {
    case MSG_START:
        scroll_pos = 0;
        selected_item = -1;
        break;

    case MSG_DRAW:
        /* Draw list box background and border */
        rectfill(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, d->bg);
        rect(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, d->fg);

        /* Draw scrollbar if needed */
        if (item_count > visible_items) {
            int scrollbar_width = 12;
            int scrollbar_x = d->x + d->w - scrollbar_width;
            int thumb_height = (visible_items * d->h) / item_count;
            if (thumb_height < 8) thumb_height = 8;
            int thumb_y = d->y + (scroll_pos * d->h) / item_count;

            rectfill(screen, scrollbar_x, d->y, d->x + d->w - 1, d->y + d->h - 1, 8);
            rect(screen, scrollbar_x, d->y, d->x + d->w - 1, d->y + d->h - 1, 15);

            /* Draw scroll thumb */
            rectfill(screen, scrollbar_x + 1, thumb_y, d->x + d->w - 2,
                thumb_y + thumb_height - 1, 7);
        }

        /* Draw visible items */
        for (i = 0; i < visible_items; i++) {
            int item_index = scroll_pos + i;
            int item_y;
            const char* item_text;

            if (item_index >= item_count) break;

            item_y = d->y + (i * 10);

            /* Highlight selected item */
            if (item_index == selected_item) {
                rectfill(screen, d->x + 1, item_y, d->x + d->w - 13, item_y + 9, 1); // Blue highlight
            }

            /* Get item text safely */
            item_text = d_list_get_item_text(d->dp, use_getter, items, item_index, item_count);
            if (!item_text) {
                item_text = "(null)";
            }

            /* Draw item text */
            textout(screen, font, item_text, d->x + 2, item_y + 1,
                (item_index == selected_item) ? 15 : d->fg);
        }
        break;

    case MSG_CLICK:
        if (mouse_x >= d->x && mouse_x < d->x + d->w &&
            mouse_y >= d->y && mouse_y < d->y + d->h) {

            /* Check if click is on scrollbar */
            if (item_count > visible_items && mouse_x >= d->x + d->w - 12) {
                /* Handle scrollbar click */
                int click_rel_y = mouse_y - d->y;
                scroll_pos = (click_rel_y * item_count) / d->h;
                if (scroll_pos < 0) scroll_pos = 0;
                if (scroll_pos > item_count - visible_items)
                    scroll_pos = item_count - visible_items;
                return D_REDRAWME;
            }
            else {
                /* Click on list item */
                int clicked_item = scroll_pos + ((mouse_y - d->y) / 10);
                if (clicked_item < item_count) {
                    selected_item = clicked_item;
                    d->d1 = selected_item; /* Store selection */
                    return D_REDRAWME;
                }
            }
        }
        break;

    case MSG_WHEEL:
        /* Mouse wheel scrolling */
        scroll_pos -= c; /* c is wheel delta */
        if (scroll_pos < 0) scroll_pos = 0;
        if (scroll_pos > item_count - visible_items)
            scroll_pos = item_count - visible_items;
        return D_REDRAWME;

    case MSG_KEY:
        if (c == (SDL_SCANCODE_UP << 8)) {
            /* Up arrow */
            if (selected_item > 0) {
                selected_item--;
                if (selected_item < scroll_pos) scroll_pos = selected_item;
                d->d1 = selected_item;
                return D_REDRAWME;
            }
        }
        else if (c == (SDL_SCANCODE_DOWN << 8)) {
            /* Down arrow */
            if (selected_item < item_count - 1) {
                selected_item++;
                if (selected_item >= scroll_pos + visible_items) {
                    scroll_pos = selected_item - visible_items + 1;
                }
                d->d1 = selected_item;
                return D_REDRAWME;
            }
        }
        else if (c == (SDL_SCANCODE_RETURN << 8)) {
            /* Enter key - select current item */
            if (selected_item >= 0 && (d->flags & D_EXIT)) {
                return D_CLOSE;
            }
        }
        break;

    case MSG_GOTFOCUS:
    case MSG_LOSTFOCUS:
        return D_REDRAWME;
    }

    return D_O_K;
}

int d_slider_proc(int msg, DIALOG* d, int c) {
    static int dragging = 0;
    static int drag_dialog = -1;  // Track which dialog is being dragged
    int slider_pos;
    int slider_width = 8;

    // Calculate slider position based on d1 value
    int range = d->d2;  // d2 is the maximum value (e.g., 63 for RGB)
    if (range <= 0) range = 100;

    int value = d->d1;  // d1 is the current value
    if (value > range) value = range;
    if (value < 0) value = 0;

    // For vertical sliders, calculate position from top
    slider_pos = d->y + ((range - value) * (d->h - slider_width)) / range;

    switch (msg) {
    case MSG_DRAW:
        // Draw background first
        rectfill(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, d->bg);

        // Draw slider track (vertical)
        rectfill(screen, d->x + (d->w / 2) - 2, d->y,
            d->x + (d->w / 2) + 1, d->y + d->h - 1, GUI_MID);
        rect(screen, d->x + (d->w / 2) - 2, d->y,
            d->x + (d->w / 2) + 1, d->y + d->h - 1, GUI_FORE);

        // Draw slider thumb
        rectfill(screen, d->x, slider_pos,
            d->x + d->w - 1, slider_pos + slider_width - 1, GUI_DESELECT);
        rect(screen, d->x, slider_pos,
            d->x + d->w - 1, slider_pos + slider_width - 1, d->fg);
        break;

    case MSG_CLICK:
    case MSG_LPRESS:
        if (mouse_x >= d->x && mouse_x < d->x + d->w &&
            mouse_y >= d->y && mouse_y < d->y + d->h) {
            dragging = 1;
            drag_dialog = (int)(d - (DIALOG*)0);  // Store dialog pointer offset

            // Update position immediately on click
            int click_rel_y = mouse_y - d->y;
            int new_value = range - ((click_rel_y * range) / d->h);
            new_value = (new_value < 0) ? 0 : ((new_value > range) ? range : new_value);

            if (new_value != d->d1) {
                d->d1 = new_value;

                // Call the callback function if it exists
                if (d->dp2) {
                    int (*callback)(void*, int) = (int (*)(void*, int))d->dp2;
                    callback(d->dp3, d->d1);
                }

                return D_REDRAWME;
            }
        }
        break;

    case MSG_IDLE:
        // Check if this dialog is being dragged
        if (dragging && (mouse_b & 1)) {
            // Check if mouse is still reasonably near this slider
            if (mouse_x >= d->x - 20 && mouse_x < d->x + d->w + 20 &&
                mouse_y >= d->y - 20 && mouse_y < d->y + d->h + 20) {

                int click_rel_y = mouse_y - d->y;
                if (click_rel_y < 0) click_rel_y = 0;
                if (click_rel_y > d->h) click_rel_y = d->h;

                int new_value = range - ((click_rel_y * range) / d->h);
                new_value = (new_value < 0) ? 0 : ((new_value > range) ? range : new_value);

                if (new_value != d->d1) {
                    d->d1 = new_value;

                    // Call the callback function if it exists
                    if (d->dp2) {
                        int (*callback)(void*, int) = (int (*)(void*, int))d->dp2;
                        callback(d->dp3, d->d1);
                    }

                    return D_REDRAWME;
                }
            }
        }
        else if (dragging && !(mouse_b & 1)) {
            // Mouse button released
            dragging = 0;
            drag_dialog = -1;
        }
        break;

    case MSG_LRELEASE:
        dragging = 0;
        drag_dialog = -1;
        break;

    case MSG_KEY:
        if (c == (SDL_SCANCODE_UP << 8)) {
            // Up arrow - increase value
            if (value < range) {
                d->d1 = value + 1;

                if (d->dp2) {
                    int (*callback)(void*, int) = (int (*)(void*, int))d->dp2;
                    callback(d->dp3, d->d1);
                }

                return D_REDRAWME;
            }
        }
        else if (c == (SDL_SCANCODE_DOWN << 8)) {
            // Down arrow - decrease value
            if (value > 0) {
                d->d1 = value - 1;

                if (d->dp2) {
                    int (*callback)(void*, int) = (int (*)(void*, int))d->dp2;
                    callback(d->dp3, d->d1);
                }

                return D_REDRAWME;
            }
        }
        break;
    }

    return D_O_K;
}

int file_select(const char* title, char* path, const char* ext) {
    // Simple file selector - in a real implementation, use native dialog
    printf("File select: %s\n", title);
    printf("Path: %s\n", path);
    printf("Ext: %s\n", ext);
    return 1; // Assume user selected a file
}

int exists(const char* filename) {
    DWORD attrib = GetFileAttributesA(filename);
    if (attrib == INVALID_FILE_ATTRIBUTES) {
        DWORD error = GetLastError();
        printf("    FileAccessError: %s - Error code: %lu\n", filename, error);
        return 0;
    }

    if (attrib & FILE_ATTRIBUTE_DIRECTORY) {
        printf("    PathIsDirectory: %s\n", filename);
        return 0;
    }

    printf("    FileExists: %s\n", filename);
    return 1;
}

long file_size(const char* filename) {
    printf("    Getting file size for: %s\n", filename);

    HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        printf("    Cannot open file: %s - Error: %lu\n", filename, error);
        return -1;
    }

    DWORD size = GetFileSize(hFile, NULL);
    CloseHandle(hFile);

    if (size == INVALID_FILE_SIZE) {
        printf("    Error getting file size for: %s\n", filename);
        return -1;
    }

    printf("    File size: %lu bytes\n", size);
    return (long)size;
}

char* get_filename(const char* path) {
    char* filename = strrchr(path, '/');
    if (!filename) filename = strrchr(path, '\\');
    return filename ? (filename + 1) : (char*)path;
}

void put_backslash(char* path) {
    int len = strlen(path);
    if (len > 0 && path[len - 1] != '/' && path[len - 1] != '\\') {
        strcat(path, "/");
    }
}

// Simple BMP loader for 8-bit BMPs (stub - you can expand this)
MYBITMAP* load_bmp(const char* filename, PALETTE pal) {
    printf("BMP loading not fully implemented, trying PCX fallback\n");

    // Try the same filename but with .pcx extension
    char pcx_filename[256];
    strncpy(pcx_filename, filename, sizeof(pcx_filename) - 1);
    char* dot = strrchr(pcx_filename, '.');
    if (dot) {
        strcpy(dot, ".pcx");
    }
    else {
        strcat(pcx_filename, ".pcx");
    }

    printf("Trying %s instead\n", pcx_filename);
    return load_pcx(pcx_filename, pal);
}

// General bitmap loader that tries different formats
MYBITMAP* load_bitmap(const char* filename, PALETTE pal) {
    printf("Loading bitmap: %s\n", filename);

    // Try different file formats
    const char* ext = strrchr(filename, '.');
    if (ext) {
        ext++; // Skip the dot

        if (_stricmp(ext, "pcx") == 0) {
            return load_pcx(filename, pal);
        }
        else if (_stricmp(ext, "bmp") == 0) {
            return load_bmp(filename, pal);
        }
        else {
            printf("Unsupported format: %s, trying PCX\n", ext);
            // Fall back to PCX
            return load_pcx(filename, pal);
        }
    }
    else {
        // No extension, try PCX
        printf("No file extension, trying PCX format\n");
        return load_pcx(filename, pal);
    }
}

// Load 8-bit (256 color) PCX
MYBITMAP* load_8bit_pcx(FILE* file, int width, int height, int bytes_per_line, PALETTE pal) {
    MYBITMAP* bmp = create_bitmap(width, height);
    if (!bmp) return NULL;

    clear_to_color(bmp, 0);

    // Decode RLE
    int x = 0, y = 0;
    unsigned char byte;

    while (y < height && fread(&byte, 1, 1, file) == 1) {
        if ((byte & 0xC0) == 0xC0) {
            int count = byte & 0x3F;
            if (fread(&byte, 1, 1, file) != 1) break;

            for (int i = 0; i < count && y < height; i++) {
                if (x < width) putpixel(bmp, x, y, byte);
                x++;
                if (x >= bytes_per_line) { x = 0; y++; }
            }
        }
        else {
            if (x < width) putpixel(bmp, x, y, byte);
            x++;
            if (x >= bytes_per_line) { x = 0; y++; }
        }
    }

    // Load palette
    fseek(file, -768, SEEK_END);
    unsigned char palette_data[768];
    if (fread(palette_data, 1, 768, file) == 768 && palette_data[0] == 12) {
        printf("Loading 256-color palette\n");
        for (int i = 0; i < 256; i++) {
            pal[i].r = palette_data[i * 3 + 0] / 4;
            pal[i].g = palette_data[i * 3 + 1] / 4;
            pal[i].b = palette_data[i * 3 + 2] / 4;
        }
        set_palette(pal);
    }

    printf("Loaded 8-bit PCX: %dx%d\n", width, height);
    return bmp;
}

// Load 1-bit (monochrome) PCX files
MYBITMAP* load_1bit_pcx(FILE* file, int width, int height, int bytes_per_line, int color_planes, PALETTE pal) {
    MYBITMAP* bmp = create_bitmap(width, height);
    if (!bmp) return NULL;

    clear_to_color(bmp, 0);

    // For 1-bit PCX, we need to handle multiple planes
    // Each plane represents a bit in the final color index
    int plane_size = bytes_per_line * height;

    // Allocate buffer for plane data
    unsigned char* plane_data = (unsigned char*)malloc(bytes_per_line);
    if (!plane_data) {
        destroy_bitmap(bmp);
        return NULL;
    }

    // Decode RLE for each scanline
    for (int y = 0; y < height; y++) {
        // Reset plane data
        memset(plane_data, 0, bytes_per_line);

        // Decode RLE for this scanline across all planes
        int bytes_decoded = 0;
        for (int plane = 0; plane < color_planes && bytes_decoded < bytes_per_line; plane++) {
            int x_pos = 0;
            while (x_pos < bytes_per_line) {
                unsigned char byte;
                if (fread(&byte, 1, 1, file) != 1) break;

                if ((byte & 0xC0) == 0xC0) {
                    // RLE run
                    int count = byte & 0x3F;
                    if (fread(&byte, 1, 1, file) != 1) break;

                    for (int i = 0; i < count && x_pos < bytes_per_line; i++) {
                        // Set the appropriate bit in the plane data
                        if (byte & (1 << (7 - (x_pos % 8)))) {
                            plane_data[x_pos] |= (1 << plane);
                        }
                        x_pos++;
                    }
                }
                else {
                    // Single byte
                    if (byte & (1 << (7 - (x_pos % 8)))) {
                        plane_data[x_pos] |= (1 << plane);
                    }
                    x_pos++;
                }
            }
            bytes_decoded = x_pos;
        }

        // Convert plane data to pixels
        for (int x = 0; x < width; x++) {
            int byte_pos = x / 8;
            int bit_pos = 7 - (x % 8);

            if (byte_pos < bytes_per_line) {
                int color_index = 0;
                for (int plane = 0; plane < color_planes; plane++) {
                    if (plane_data[byte_pos] & (1 << plane)) {
                        color_index |= (1 << plane);
                    }
                }
                putpixel(bmp, x, y, color_index);
            }
        }
    }

    free(plane_data);

    // Set up a default palette for 1-bit images
    if (color_planes == 1) {
        // Monochrome: black and white
        pal[0].r = 0; pal[0].g = 0; pal[0].b = 0;
        pal[1].r = 63; pal[1].g = 63; pal[1].b = 63;
    }
    else if (color_planes == 2) {
        // 4-color CGA-style palette
        pal[0].r = 0; pal[0].g = 0; pal[0].b = 0;       // Black
        pal[1].r = 63; pal[1].g = 0; pal[1].b = 0;      // Red
        pal[2].r = 0; pal[2].g = 63; pal[2].b = 0;      // Green  
        pal[3].r = 63; pal[3].g = 63; pal[3].b = 0;     // Yellow
    }
    else if (color_planes == 4) {
        // 16-color EGA palette
        // Basic EGA colors
        int ega_colors[16][3] = {
            {0, 0, 0}, {0, 0, 42}, {0, 42, 0}, {0, 42, 42},
            {42, 0, 0}, {42, 0, 42}, {42, 21, 0}, {42, 42, 42},
            {21, 21, 21}, {21, 21, 63}, {21, 63, 21}, {21, 63, 63},
            {63, 21, 21}, {63, 21, 63}, {63, 63, 21}, {63, 63, 63}
        };

        for (int i = 0; i < 16; i++) {
            pal[i].r = ega_colors[i][0];
            pal[i].g = ega_colors[i][1];
            pal[i].b = ega_colors[i][2];
        }
    }

    printf("Loaded 1-bit PCX: %dx%d, %d planes (%d colors)\n",
        width, height, color_planes, 1 << color_planes);
    return bmp;
}

// Save PCX file (8-bit only for now)
void save_pcx(const char* filename, MYBITMAP* bmp, PALETTE pal) {
    if (!bmp || !filename) {
        printf("ERROR: Cannot save PCX - invalid parameters\n");
        return;
    }

    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("ERROR: Cannot create PCX file %s\n", filename);
        return;
    }

    // Create PCX header
    PCX_HEADER header;
    memset(&header, 0, sizeof(PCX_HEADER));

    header.manufacturer = 0x0A;
    header.version = 5;        // PC Paintbrush 3.0+
    header.encoding = 1;       // RLE encoding
    header.bits_per_pixel = 8;
    header.xmin = 0;
    header.ymin = 0;
    header.xmax = bmp->w - 1;
    header.ymax = bmp->h - 1;
    header.hres = bmp->w;
    header.vres = bmp->h;
    header.color_planes = 1;   // 8-bit = 1 plane
    header.bytes_per_line = bmp->w;

    // Write header
    if (fwrite(&header, sizeof(PCX_HEADER), 1, file) != 1) {
        printf("ERROR: Failed to write PCX header\n");
        fclose(file);
        return;
    }

    // RLE encode and write image data
    for (int y = 0; y < bmp->h; y++) {
        int x = 0;
        while (x < bmp->w) {
            int current_color = getpixel(bmp, x, y);
            int run_length = 1;

            // Count run of same color
            while (x + run_length < bmp->w &&
                getpixel(bmp, x + run_length, y) == current_color &&
                run_length < 63) {
                run_length++;
            }

            if (run_length > 1 || (current_color & 0xC0) == 0xC0) {
                // Use RLE encoding
                unsigned char rle_byte = 0xC0 | run_length;
                fwrite(&rle_byte, 1, 1, file);
                fwrite(&current_color, 1, 1, file);
            }
            else {
                // Single byte
                fwrite(&current_color, 1, 1, file);
            }

            x += run_length;
        }
    }

    // Write palette marker and palette
    unsigned char palette_marker = 12;
    fwrite(&palette_marker, 1, 1, file);

    // Write 256-color palette (convert from 6-bit to 8-bit)
    for (int i = 0; i < 256; i++) {
        unsigned char r = (pal[i].r * 255) / 63;
        unsigned char g = (pal[i].g * 255) / 63;
        unsigned char b = (pal[i].b * 255) / 63;
        fwrite(&r, 1, 1, file);
        fwrite(&g, 1, 1, file);
        fwrite(&b, 1, 1, file);
    }

    fclose(file);
    printf("Saved PCX: %s (%dx%d, 256 colors)\n", filename, bmp->w, bmp->h);
}

// Separate PCX loader
MYBITMAP* load_pcx(const char* filename, PALETTE pal) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("ERROR: Cannot open PCX file %s\n", filename);
        return NULL;
    }

    PCX_HEADER header;
    if (fread(&header, sizeof(PCX_HEADER), 1, file) != 1) {
        printf("ERROR: Cannot read PCX header\n");
        fclose(file);
        return NULL;
    }

    if (header.manufacturer != 0x0A) {
        printf("ERROR: Not a valid PCX file\n");
        fclose(file);
        return NULL;
    }

    int width = header.xmax - header.xmin + 1;
    int height = header.ymax - header.ymin + 1;
    int bytes_per_line = header.bytes_per_line;

    printf("PCX: %dx%d, encoding: %d, bpp: %d, planes: %d\n",
        width, height, header.encoding, header.bits_per_pixel, header.color_planes);

    // Handle different PCX formats
    MYBITMAP* bmp = NULL;

    if (header.bits_per_pixel == 8 && header.color_planes == 1) {
        bmp = load_8bit_pcx(file, width, height, bytes_per_line, pal);
    }
    else if (header.bits_per_pixel == 1 && header.color_planes <= 4) {
        bmp = load_1bit_pcx(file, width, height, bytes_per_line, header.color_planes, pal);
    }
    else {
        printf("ERROR: Unsupported PCX format (%d bpp, %d planes)\n",
            header.bits_per_pixel, header.color_planes);
    }

    fclose(file);
    return bmp;
}

// Config state management
static char current_config_file[256] = "";
static char* config_data = NULL;
static size_t config_size = 0;

void push_config_state(void) {
    // Save current config state if needed
    // For now, just a stub since we're only using one config file at a time
}

void pop_config_state(void) {
    // Restore previous config state
    // For now, just clear the current config
    if (config_data) {
        free(config_data);
        config_data = NULL;
    }
    current_config_file[0] = '\0';
}

void set_config_file(const char* filename) {
    // Clear previous config
    if (config_data) {
        free(config_data);
        config_data = NULL;
    }

    strncpy(current_config_file, filename, sizeof(current_config_file) - 1);

    // Load the file
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Cannot open config file: %s\n", filename);
        return;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    config_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory
    config_data = malloc(config_size + 1);
    if (!config_data) {
        fclose(file);
        return;
    }

    // Read file
    size_t read = fread(config_data, 1, config_size, file);
    config_data[read] = '\0'; // Null terminate

    fclose(file);
}

// Helper function to trim whitespace - improved version
char* trim(char* str) {
    if (!str) return str;

    char* end;

    // Trim leading space
    while (*str == ' ' || *str == '\t' || *str == '\r' || *str == '\n') {
        str++;
    }

    if (*str == 0) return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) {
        end--;
    }

    end[1] = '\0';
    return str;
}


void set_config_string(const char* section, const char* name, const char* value) {
    printf("Config: [%s] %s = %s\n", section, name, value);

    if (!config_data) {
        printf("ERROR: No config data loaded\n");
        return;
    }

    // Create a new config string with the updated value
    char new_config[4096] = "";  // Adjust size as needed
    char* data = config_data;
    int in_target_section = 0;
    int key_found = 0;
    char line[256];

    while (*data) {
        // Read line
        char* line_start = data;
        while (*data && *data != '\n' && *data != '\r') {
            data++;
        }

        // Copy line
        size_t line_len = data - line_start;
        if (line_len >= sizeof(line)) line_len = sizeof(line) - 1;
        memcpy(line, line_start, line_len);
        line[line_len] = '\0';

        // Skip line endings
        while (*data == '\n' || *data == '\r') {
            data++;
        }

        char* trimmed_line = trim(line);
        char* original_line = line; // Keep original for copying

        // Skip empty lines and comments
        if (trimmed_line[0] == '\0' || trimmed_line[0] == ';' || trimmed_line[0] == '#') {
            strcat(new_config, original_line);
            strcat(new_config, "\n");
            continue;
        }

        // Check for section
        if (trimmed_line[0] == '[' && trimmed_line[strlen(trimmed_line) - 1] == ']') {
            char section_name[64];
            strncpy(section_name, trimmed_line + 1, strlen(trimmed_line) - 2);
            section_name[strlen(trimmed_line) - 2] = '\0';
            trim(section_name);

            // If we were in the target section and didn't find the key, add it before leaving
            if (in_target_section && !key_found) {
                char new_entry[256];
                sprintf(new_entry, "%s=%s\n", name, value);
                strcat(new_config, new_entry);
                key_found = 1;
            }

            in_target_section = (_stricmp(section_name, section) == 0);
            strcat(new_config, original_line);
            strcat(new_config, "\n");
            continue;
        }

        // If we're in the target section, look for the key
        if (in_target_section) {
            char* equals = strchr(trimmed_line, '=');
            if (equals) {
                *equals = '\0';
                char* key = trim(trimmed_line);

                if (_stricmp(key, name) == 0) {
                    // Found the key - replace the value
                    char new_line[256];
                    sprintf(new_line, "%s=%s", name, value);
                    strcat(new_config, new_line);
                    strcat(new_config, "\n");
                    key_found = 1;
                    continue;
                }
                *equals = '='; // Restore the equals sign
            }
        }

        // Copy the original line
        strcat(new_config, original_line);
        strcat(new_config, "\n");
    }

    // If we never found the section or key, add them at the end
    if (!key_found) {
        // Add section if it doesn't exist
        int section_exists = 0;
        char* search = config_data;
        char search_section[128];
        sprintf(search_section, "[%s]", section);

        while (*search) {
            if (strstr(search, search_section)) {
                section_exists = 1;
                break;
            }
            search++;
        }

        if (!section_exists) {
            strcat(new_config, search_section);
            strcat(new_config, "\n");
        }

        // Add the key-value pair
        char new_entry[256];
        sprintf(new_entry, "%s=%s\n", name, value);
        strcat(new_config, new_entry);
    }

    // Replace the old config data
    free(config_data);
    config_data = strdup(new_config);
    config_size = strlen(config_data);

    // Write to file
    if (strlen(current_config_file) > 0) {
        FILE* file = fopen(current_config_file, "w");
        if (file) {
            fwrite(config_data, 1, config_size, file);
            fclose(file);
            printf("Config saved to: %s\n", current_config_file);
        }
        else {
            printf("ERROR: Could not write config file: %s\n", current_config_file);
        }
    }
}

void set_config_int(const char* section, const char* name, int value) {
    char value_str[32];
    sprintf(value_str, "%d", value);
    set_config_string(section, name, value_str);
}

char* get_config_string(const char* section, const char* name, const char* default_value) {
    if (!config_data) {
        printf("No config data loaded, returning default: %s\n", default_value);
        return (char*)default_value;
    }

    char* data = config_data;
    int in_target_section = 0;
    char line[256];

    while (*data) {
        // Read line
        char* line_start = data;
        while (*data && *data != '\n' && *data != '\r') {
            data++;
        }

        // Copy line
        size_t line_len = data - line_start;
        if (line_len >= sizeof(line)) line_len = sizeof(line) - 1;
        memcpy(line, line_start, line_len);
        line[line_len] = '\0';

        // Skip line endings
        while (*data == '\n' || *data == '\r') {
            data++;
        }

        char* trimmed_line = trim(line);

        // Skip empty lines and comments
        if (trimmed_line[0] == '\0' || trimmed_line[0] == ';' || trimmed_line[0] == '#') {
            continue;
        }

        // Check for section
        if (trimmed_line[0] == '[' && trimmed_line[strlen(trimmed_line) - 1] == ']') {
            char section_name[64];
            strncpy(section_name, trimmed_line + 1, strlen(trimmed_line) - 2);
            section_name[strlen(trimmed_line) - 2] = '\0';
            trim(section_name);

            in_target_section = (_stricmp(section_name, section) == 0);
            continue;
        }

        // If we're in the target section, look for the key
        if (in_target_section) {
            char* equals = strchr(trimmed_line, '=');
            if (equals) {
                *equals = '\0';
                char* key = trim(trimmed_line);
                char* value = trim(equals + 1);

                // Use _stricmp instead of strcasecmp for Windows
                if (_stricmp(key, name) == 0) {
                    return _strdup(value); // Use _strdup for Windows
                }
            }
        }
    }

    return (char*)default_value;
}

int get_config_int(const char* section, const char* name, int default_value) {
    char* str_value = get_config_string(section, name, NULL);
    if (str_value == NULL) {
        return default_value;
    }

    // Check if the string is empty or invalid
    if (strlen(str_value) == 0) {
        return default_value;
    }

    int result = atoi(str_value);

    // Only free if it was allocated by get_config_string (not the default)
    if (str_value != (char*)NULL && str_value != (char*)default_value) {
        free(str_value);
    }

    return result;
}

// Completely rewritten palette parser
char** parse_palette_values(char* str_value, int* argc) {
    // Make a working copy we can modify
    char* work_str = strdup(str_value);
    if (!work_str) {
        *argc = 0;
        return NULL;
    }

    // First pass: count how many values we have
    int count = 0;
    char* ptr = work_str;
    int in_number = 0;

    while (*ptr) {
        if (*ptr == ',' || *ptr == ' ') {
            if (in_number) {
                count++;
                in_number = 0;
            }
        }
        else {
            in_number = 1;
        }
        ptr++;
    }
    if (in_number) {
        count++;
    }

    // Allocate result array
    char** argv = (char**)malloc(count * sizeof(char*));
    if (!argv) {
        free(work_str);
        *argc = 0;
        return NULL;
    }

    // Second pass: extract values
    int index = 0;
    ptr = work_str;
    char* start = NULL;

    while (*ptr && index < count) {
        if (*ptr != ',' && *ptr != ' ') {
            if (!start) {
                start = ptr; // Start of a new value
            }
        }
        else {
            if (start) {
                // End of a value
                int len = ptr - start;
                argv[index] = (char*)malloc(len + 1);
                strncpy(argv[index], start, len);
                argv[index][len] = '\0';
                index++;
                start = NULL;
            }
        }
        ptr++;
    }

    // Don't forget the last value
    if (start && index < count) {
        int len = ptr - start;
        argv[index] = (char*)malloc(len + 1);
        strncpy(argv[index], start, len);
        argv[index][len] = '\0';
        index++;
    }

    *argc = index;
    free(work_str);
    return argv;
}

// Special parser for GraphicsRoms entries (format: "start length filename")
char** parse_graphics_roms(char* str_value, int* argc) {
    printf("Parsing GraphicsRoms entry: %s\n", str_value);

    // GraphicsRoms should have exactly 3 parts: start, length, filename
    char** argv = (char**)malloc(3 * sizeof(char*));
    if (!argv) {
        *argc = 0;
        return NULL;
    }

    int index = 0;
    char* start = str_value;

    // Parse first token (start address)
    while (*start == ' ') start++;
    char* end = start;
    while (*end && *end != ' ') end++;
    if (start != end) {
        int len = end - start;
        argv[index] = (char*)malloc(len + 1);
        strncpy(argv[index], start, len);
        argv[index][len] = '\0';
        argv[index] = trim(argv[index]);
        index++;
    }

    // Parse second token (length)
    start = end;
    while (*start == ' ') start++;
    end = start;
    while (*end && *end != ' ') end++;
    if (start != end) {
        int len = end - start;
        argv[index] = (char*)malloc(len + 1);
        strncpy(argv[index], start, len);
        argv[index][len] = '\0';
        argv[index] = trim(argv[index]);
        printf("  rom[%d] = '%s' (length)\n", index, argv[index]);
        index++;
    }

    // The rest is the filename (may contain spaces)
    start = end;
    while (*start == ' ') start++;
    if (*start) {
        argv[index] = strdup(start);
        argv[index] = trim(argv[index]);
        printf("  rom[%d] = '%s' (filename)\n", index, argv[index]);
        index++;
    }

    *argc = index;
    printf("GraphicsRoms parsed into %d parts\n", index);
    return argv;
}

// Default parser for other values (space-separated)
char** parse_default_values(char* str_value, int* argc) {
    printf("Parsing default values: %s\n", str_value);

    // Count space-separated tokens
    int max_tokens = 1;
    char* temp = str_value;
    while (*temp) {
        if (*temp == ' ') max_tokens++;
        temp++;
    }

    // Allocate array for tokens
    char** argv = (char**)malloc(max_tokens * sizeof(char*));
    if (!argv) {
        *argc = 0;
        return NULL;
    }

    int index = 0;
    char* token = strtok(str_value, " ");
    while (token != NULL && index < max_tokens) {
        char* trimmed = trim(token);
        if (strlen(trimmed) > 0) {
            argv[index] = strdup(trimmed);
            printf("  default[%d] = '%s'\n", index, argv[index]);
            index++;
        }
        token = strtok(NULL, " ");
    }

    *argc = index;
    printf("Default values parsed into %d tokens\n", index);
    return argv;
}

char** get_config_argv(const char* section, const char* name, int* argc) {
    if (!config_data) {
        *argc = 0;
        return NULL;
    }

    // First get the string value
    char* str_value = get_config_string(section, name, NULL);
    if (str_value == NULL || str_value == (char*)NULL) {
        printf("ERROR: Could not get config string\n");
        *argc = 0;
        return NULL;
    }

    // Special handling for different sections
    if (strcmp(section, "Palette") == 0) {
        return parse_palette_values(str_value, argc);
    }
    else if (strcmp(section, "GraphicsRoms") == 0) {
        return parse_graphics_roms(str_value, argc);
    }
    else {
        return parse_default_values(str_value, argc);
    }
}

void simulate_keypress(int key) {
    // Simulate a key press
}

// Text functions
// Replace textprintf and textprintf_centre with unified versions
void textprintf_ex(MYBITMAP* bmp, FONT* font, int x, int y, int color, int center, const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    if (center) {
        textout_centre(bmp, font, buffer, x, y, color);
    }
    else {
        textout(bmp, font, buffer, x, y, color);
    }
}


int text_height(FONT* font) {
    return font ? font->height : 8;
}

// Mouse cursor management
static SDL_Cursor* custom_cursor = NULL;
static int cursor_focus_x = 0;
static int cursor_focus_y = 0;

void set_mouse_sprite(MYBITMAP* sprite) {
    // Free previous custom cursor
    if (custom_cursor) {
        SDL_FreeCursor(custom_cursor);
        custom_cursor = NULL;
    }

    if (!sprite) {
        // NULL sprite means use default cursor
        SDL_ShowCursor(SDL_ENABLE);
        return;
    }

    // Convert MYBITMAP to SDL surface for cursor
    SDL_Surface* cursor_surface = SDL_CreateRGBSurface(0, sprite->w, sprite->h, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!cursor_surface) {
        printf("Failed to create cursor surface: %s\n", SDL_GetError());
        return;
    }

    // Convert 8-bit palette-based bitmap to 32-bit RGBA
    // Note: This assumes color 0 is transparent
    for (int y = 0; y < sprite->h; y++) {
        for (int x = 0; x < sprite->w; x++) {
            int color_index = getpixel(sprite, x, y);
            Uint32 pixel_color;

            if (color_index == 0) {
                // Transparent
                pixel_color = 0x00000000;
            }
            else {
                // Use palette color
                RGB color = desktop_palette[color_index];
                Uint8 r = (color.r * 255) / 63;
                Uint8 g = (color.g * 255) / 63;
                Uint8 b = (color.b * 255) / 63;
                pixel_color = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }

            // Set pixel (this is slow but cursers are small)
            Uint32* pixels = (Uint32*)cursor_surface->pixels;
            pixels[y * cursor_surface->w + x] = pixel_color;
        }
    }

    // Create cursor from surface
    custom_cursor = SDL_CreateColorCursor(cursor_surface, cursor_focus_x, cursor_focus_y);
    SDL_FreeSurface(cursor_surface);

    if (custom_cursor) {
        SDL_SetCursor(custom_cursor);
        SDL_ShowCursor(SDL_ENABLE);
    }
    else {
        printf("Failed to create custom cursor: %s\n", SDL_GetError());
    }
}

void set_mouse_sprite_focus(int x, int y) {
    cursor_focus_x = x;
    cursor_focus_y = y;

    // If we have a custom cursor, we'd need to recreate it with the new hotspot
    // For simplicity, we'll just store the values for when the cursor is set
}

int mouse_range_min_x = 0;
int mouse_range_min_y = 0;
int mouse_range_max_x = 319;
int mouse_range_max_y = 199;

void set_mouse_range(int x1, int y1, int x2, int y2) {
    // Store the bounds
    mouse_range_min_x = x1;
    mouse_range_min_y = y1;
    mouse_range_max_x = x2;
    mouse_range_max_y = y2;
}

// Other functions
void fade_out(int speed) {
    // Fade screen to black
    for (int i = 63; i >= 0; i--) {
        // Adjust palette colors here
        SDL_Delay(speed);
    }
}

fixed itofix(int x) {
    return x << 16;  // Convert integer to fixed point
}

// Lock functions (stubs for DOS compatibility)
void LOCK_FUNCTION(void* x) {
    // No-op in Windows/SDL
}

void LOCK_VARIABLE(int x) {
    // No-op in Windows/SDL
}

void SDL_Flip(void) {
    static int frame_count = 0;
    frame_count++;

    if (!sdl_window || !sdl_renderer || !sdl_texture || !sdl_pixels || !screen || !screen->dat) {
        return;
    }

    // Update palette if dirty
    if (palette_dirty) {
        for (int i = 0; i < 256; i++) {
            RGB color = desktop_palette[i];
            Uint8 r = (color.r * 255) / 63;
            Uint8 g = (color.g * 255) / 63;
            Uint8 b = (color.b * 255) / 63;
            palette_rgb[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
        palette_dirty = 0;
    }

    // Convert 8-bit to 32-bit
    Uint32* dst = sdl_pixels;
    unsigned char* src = (unsigned char*)screen->dat;
    int pixels = screen->w * screen->h;

    for (int i = 0; i < pixels; i++) {
        int color_index = src[i];
        dst[i] = palette_rgb[color_index & 0xFF];
    }

    // Update and render
    SDL_UpdateTexture(sdl_texture, NULL, dst, screen->w * sizeof(Uint32));
    SDL_RenderClear(sdl_renderer);
    SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
    SDL_RenderPresent(sdl_renderer);
}

// Update mouse state from SDL events
void update_mouse_state(SDL_Event* event) {
    switch (event->type) {
    case SDL_MOUSEMOTION:
        mouse_x = event->motion.x;
        mouse_y = event->motion.y;

        // Constrain mouse to range if set
        if (mouse_range_max_x > mouse_range_min_x && mouse_range_max_y > mouse_range_min_y) {
            if (mouse_x < mouse_range_min_x) mouse_x = mouse_range_min_x;
            if (mouse_x > mouse_range_max_x) mouse_x = mouse_range_max_x;
            if (mouse_y < mouse_range_min_y) mouse_y = mouse_range_min_y;
            if (mouse_y > mouse_range_max_y) mouse_y = mouse_range_max_y;

            // If we constrained, update the actual mouse position
            // NOTE: SDL_WarpMouseInWindow uses physical window coordinates,
            // but mouse_x/mouse_y are in logical coordinates (320x240)
            // With SDL_RenderSetLogicalSize, we need to scale to physical coords
            if (mouse_x != event->motion.x || mouse_y != event->motion.y) {
                // Calculate scale factor (physical / logical)
                int window_w, window_h;
                SDL_GetWindowSize(sdl_window, &window_w, &window_h);
                float scale_x = (float)window_w / 320.0f;
                float scale_y = (float)window_h / 240.0f;

                int physical_x = (int)(mouse_x * scale_x);
                int physical_y = (int)(mouse_y * scale_y);

                SDL_WarpMouseInWindow(sdl_window, physical_x, physical_y);
            }
        }
        break;

    case SDL_MOUSEBUTTONDOWN:
        mouse_x = event->button.x;
        mouse_y = event->button.y;
        if (event->button.button == SDL_BUTTON_LEFT) {
            mouse_b |= 1;
        }
        else if (event->button.button == SDL_BUTTON_RIGHT) {
            mouse_b |= 2;
        }
        break;

    case SDL_MOUSEBUTTONUP:
        mouse_x = event->button.x;
        mouse_y = event->button.y;
        if (event->button.button == SDL_BUTTON_LEFT) {
            mouse_b &= ~1;
        }
        else if (event->button.button == SDL_BUTTON_RIGHT) {
            mouse_b &= ~2;
        }
        break;

    case SDL_MOUSEWHEEL:
        mouse_z += event->wheel.y;
        break;
    }
}

extern MYBITMAP* current_sprite;

void initialize_default_sprite(void) {
    if (!current_sprite) {
        current_sprite = create_bitmap(8, 8);
        if (current_sprite) {
            // Initialize with a checkerboard pattern or solid color
            clear_to_color(current_sprite, 0); // Black background
        }
        else {
            printf("ERROR: Failed to create default sprite\n");
        }
    }
}

// Global input state
static int key_queue[16];
static int key_queue_head = 0;
static int key_queue_tail = 0;

int keypressed(void) {
    SDL_PumpEvents();
    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    // Check for any key pressed
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        if (keystate[i]) return 1;
    }

    return key_queue_head != key_queue_tail;
}

// Convert SDL key to Allegro-style key code
// Enhanced key mapping
int sdl_key_to_allegro(SDL_Keysym keysym) {
    // Map SDL keycodes to Allegro scancodes
    switch (keysym.sym) {
    case SDLK_RETURN: return SCANCODE_TO_KEY(28);
    case SDLK_ESCAPE: return SCANCODE_TO_KEY(1);
    case SDLK_SPACE: return SCANCODE_TO_KEY(57);
    case SDLK_BACKSPACE: return SCANCODE_TO_KEY(14);
    case SDLK_TAB: return SCANCODE_TO_KEY(15);
    case SDLK_UP: return KEY_UP;
    case SDLK_DOWN: return KEY_DOWN;
    case SDLK_LEFT: return KEY_LEFT;
    case SDLK_RIGHT: return KEY_RIGHT;
    case SDLK_HOME: return KEY_HOME;
    case SDLK_END: return KEY_END;
    case SDLK_PAGEUP: return KEY_PGUP;
    case SDLK_PAGEDOWN: return KEY_PGDN;
    case SDLK_F1: return KEY_F1;
    case SDLK_F2: return KEY_F2;
    case SDLK_F3: return KEY_F3;
    case SDLK_F4: return KEY_F4;
    case SDLK_F5: return KEY_F5;
    case SDLK_F6: return KEY_F6;
    case SDLK_F7: return KEY_F7;
    case SDLK_F8: return KEY_F8;
    case SDLK_F9: return KEY_F9;
    case SDLK_F10: return KEY_F10;
    case SDLK_F11: return KEY_F11;
    case SDLK_F12: return KEY_F12;
    default:
        if (keysym.sym >= SDLK_a && keysym.sym <= SDLK_z) {
            int base = 30; // SDLK_a should map to scancode 30 (KEY_A)
            return SCANCODE_TO_KEY(base + (keysym.sym - SDLK_a));
        }
        return SCANCODE_TO_KEY(keysym.scancode);
    }
}

int readkey(void) {
    // Return from queue if available
    if (key_queue_head != key_queue_tail) {
        int key = key_queue[key_queue_tail];
        key_queue_tail = (key_queue_tail + 1) % 16;
        return key;
    }

    // Otherwise wait for key
    SDL_Event event;
    while (SDL_WaitEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            return sdl_key_to_allegro(event.key.keysym);
        }
    }
    return 0;
}
