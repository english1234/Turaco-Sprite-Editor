// sprtplte.c
//
//  Sprite Palette UI module for allegro
//
//  September, 1998
//  jerry@mail.csh.rit.edu

// see the .h file for what data is to be stored in what fields
#include "../INCLUDE/ALLEGRO.H"
#include <stdio.h>

#include "../INCLUDE/general.h"
#include "../INCLUDE/util.h"
#include "../INCLUDE/sprtplte.h"
#include "../INCLUDE/GUIPAL.H"

extern MYBITMAP* screen;          // Main display bitmap
extern FONT* font;              // Default system font

extern volatile int mouse_x;
extern volatile int mouse_y;
extern volatile int mouse_z;

extern MYBITMAP* create_bitmap(int width, int height);
extern void text_mode(int mode);

// internal structure...
struct _sprite_palette_internals {
    // internally controlled:
    int hilite;
};


// sprite_get
//     which_one is the index.  if it is out of range, nothing happens
//     a_sprite must be big enough for the sprite.
//     sp is a pointer to the structure to pull the sprite out of
void sprite_get(int which_one, MYBITMAP * a_sprite, SPRITE_PALETTE * spm)
{
    int bmp_x; 

    // pointer checks...
    if (a_sprite == NULL) return;
    if (spm == NULL) return;
    if (spm->bmp == NULL) return;

    // bounds check - if fail, fill with the background color
    if (which_one < 0 || which_one >= spm->n_total)
    {
	clear_to_color(a_sprite, GUI_BACK);
	return;
    }
    // now copy the sprite data
    bmp_x = which_one * spm->sprite_w;
    blit(spm->bmp, a_sprite, bmp_x, 0, 0, 0, spm->sprite_w, spm->sprite_h);
}


// sprite_put
//     which one is the index.  if it is out of range, nothing happens
//     a_sprite must be big enough for the sprite.
//     sp is a pointer to the structure to put the sprite into
void sprite_put(int which_one, MYBITMAP * a_sprite, SPRITE_PALETTE * spm)
{
    int bmp_x; 

    // pointer checks...
    if (a_sprite == NULL) return;
    if (spm == NULL) return;

    // bounds check
    if (which_one < 0 || which_one > spm->n_total) return;

    // now copy the sprite data
    bmp_x = which_one * spm->sprite_w;
    blit(a_sprite, spm->bmp, 0, 0, bmp_x, 0, spm->sprite_w, spm->sprite_h);

    // and set the changed flag so we can re-draw it properly
    spm->flags |= SPRITE_FLAG_NEW;
}

// Helper function to calculate maximum visible sprites
int calculate_max_visible_sprites(SPRITE_PALETTE* spm, int width, int height) {
    if (!spm || spm->sprite_w <= 0 || spm->sprite_h <= 0)
        return 0;

    int sprites_per_row = (width - 4) / spm->sprite_w;
    int visible_rows = (height - 4) / spm->sprite_h;

    return sprites_per_row * visible_rows;
}

// Helper function to ensure first_sprite is within valid range
void clamp_first_sprite(SPRITE_PALETTE* spm, int max_visible) {
    if (!spm) return;

    int max_first = spm->n_total - max_visible;
    if (max_first < 0) max_first = 0;

    if (spm->first_sprite > max_first) {
        spm->first_sprite = max_first;
    }
    if (spm->first_sprite < 0) {
        spm->first_sprite = 0;
    }
}

int sprite_palette_proc(int msg, DIALOG* d, int c)
{
    MYBITMAP* tsprite = NULL;
    MYBITMAP* temp = NULL;
    struct _sprite_palette_internals* bi = NULL;
    struct sprite_palette** spmp = NULL;
    struct sprite_palette* spm = NULL;
    int ix, iy, ic, iymax, ixmax;
    int mouse_icx, mouse_icy;
    int cursor_icx, cursor_icy;
    int anim_icx, anim_icy;
    int retval = D_O_K;
    int (*proc)(int msg, DIALOG * dl, int sprite_number); 

    /* process the message */
    switch (msg) {

        /* initialise when we get a start message */
    case MSG_START:
        if (d->dp)
        {
            spmp = d->dp;
            spm = *spmp;
            if (spm)
                spm->flags = SPRITE_FLAG_NEW;
        }

        d->dp3 = malloc(sizeof(struct _sprite_palette_internals));
        bi = d->dp3;
        bi->hilite = COLOR_LOLITE;
        break;

        /* shutdown when we get an end message */
    case MSG_END:
        free(d->dp3);
        break;

    case MSG_ANIM_POS_SET:
        spmp = d->dp;
        spm = *spmp;
        spm->anim_pos = c;

        /* draw in response to draw messages */
    case MSG_DRAW:
        bi = d->dp3;
        spmp = d->dp;
        spm = *spmp;

        if (spm)
            if (spm->flags & SPRITE_FLAG_NEW)
                spm->flags &= ~(SPRITE_FLAG_NEW);

        temp = create_bitmap(d->w, d->h);
        clear_to_color(temp, GUI_BACK);

        mouse_icx = mouse_icy = -1;
        cursor_icx = cursor_icy = -1;
        anim_icx = anim_icy = -1;

        if (spm == NULL || spm->bmp == NULL)
        {
            text_mode(-1);
            textout_centre(temp, font, "GFX Bank Not Loaded",
                (d->w) / 2, (d->h) / 2, GUI_FORE);
        }
        else {
            if (spm->sprite_w < 257 && spm->sprite_w > 0)
            {
                tsprite = create_bitmap(spm->sprite_w, spm->sprite_h);

                // Calculate available space for sprites (leave room for scrollbar)
                int scrollbar_width = 12;
                int sprite_area_width = d->w - scrollbar_width - 4;

                ixmax = sprite_area_width / spm->sprite_w;
                iymax = (temp->h - 4) / spm->sprite_h;
                if (ixmax > 32) ixmax = 32;

                spm->n_per_row = ixmax;
                spm->n_rows = iymax;

                // Calculate if we need a scrollbar
                int total_sprites = spm->n_total;
                int max_visible_sprites = ixmax * iymax;
                int needs_scrollbar = (total_sprites > max_visible_sprites);

                if (needs_scrollbar) {
                    // Draw scrollbar on the right side
                    int scrollbar_x = sprite_area_width + 2;

                    // Draw scrollbar track
                    rectfill(temp, scrollbar_x, 2, d->w - 2, d->h - 2, GUI_MID);
                    rect(temp, scrollbar_x, 2, d->w - 2, d->h - 2, GUI_FORE);

                    // Calculate scrollbar thumb size and position
                    float visible_ratio = (float)max_visible_sprites / total_sprites;
                    int thumb_height = (d->h - 4) * visible_ratio;
                    if (thumb_height < 16) thumb_height = 16;

                    float scroll_ratio = 0.0;
                    int scroll_range = total_sprites - max_visible_sprites;
                    if (scroll_range > 0) {
                        scroll_ratio = (float)spm->first_sprite / scroll_range;
                    }

                    int thumb_y = 2 + (int)((d->h - 4 - thumb_height) * scroll_ratio);

                    // Draw scrollbar thumb
                    rectfill(temp, scrollbar_x + 1, thumb_y,
                        d->w - 3, thumb_y + thumb_height, GUI_SELECT);
                    rect(temp, scrollbar_x + 1, thumb_y,
                        d->w - 3, thumb_y + thumb_height, GUI_FORE);

                    printf("DEBUG: Scrollbar - dialog=(%d,%d,%d,%d), scrollbar_x=%d, thumb_y=%d\n",
                        d->x, d->y, d->w, d->h, scrollbar_x, thumb_y);
                }

                ic = spm->first_sprite;
                for (iy = 0; (iy < iymax) && (ic < spm->n_total); iy++)
                {
                    for (ix = 0; (ix < ixmax) && (ic < spm->n_total); ix++)
                    {
                        sprite_get(ic, tsprite, spm);
                        if (ic == spm->last_selected)
                        {
                            cursor_icx = ix * spm->sprite_w + 2;
                            cursor_icy = iy * spm->sprite_h + 2;
                        }

                        if (ic == spm->mouse_over)
                        {
                            mouse_icx = ix * spm->sprite_w + 2;
                            mouse_icy = iy * spm->sprite_h + 2;
                        }

                        if (ic == spm->anim_pos)
                        {
                            anim_icx = ix * spm->sprite_w + 2;
                            anim_icy = iy * spm->sprite_h + 2;
                        }

                        blit(tsprite, temp, 0, 0,
                            ix * spm->sprite_w + 2, iy * spm->sprite_h + 2,
                            spm->sprite_w, spm->sprite_h);

                        ic++;
                    }
                }
                destroy_bitmap(tsprite);
            }
        }

        if (cursor_icx != -1)
        {
            rect(temp,
                cursor_icx - 1, cursor_icy - 1,
                cursor_icx + spm->sprite_w, cursor_icy + spm->sprite_h,
                COLOR_HILITE);
        }

        if (mouse_icx != -1)
        {
            my_draw_dotted_rect(temp,
                mouse_icx - 1, mouse_icy - 1,
                mouse_icx + spm->sprite_w, mouse_icy + spm->sprite_h,
                COLOR_LOLITE);
        }

        if (anim_icx != -1)
        {
            my_draw_dotted_rect(temp,
                anim_icx - 1, anim_icy - 1,
                anim_icx + spm->sprite_w, anim_icy + spm->sprite_h,
                COLOR_GREEN);
        }

        box_3d(temp, temp->w, temp->h,
            (bi->hilite == COLOR_HILITE) ? 1 : 0,
            (bi->hilite == COLOR_HILITE) ? 1 : 0
        );

        blit(temp, screen, 0, 0, d->x, d->y, d->w, d->h);
        destroy_bitmap(temp);
        break;

  case MSG_CLICK:
  case MSG_DCLICK:
      printf("DEBUG: Sprite palette click - mouse=(%d,%d), dialog=(%d,%d,%d,%d)\n",
          mouse_x, mouse_y, d->x, d->y, d->w, d->h);

      spmp = d->dp;
      spm = *spmp;

      if (spm != NULL && spm->bmp)
      {
          // Calculate scrollbar position (same as in MSG_DRAW)
          int scrollbar_width = 12;
          int sprite_area_width = d->w - scrollbar_width - 4;
          int scrollbar_x = sprite_area_width + 2;

          printf("DEBUG: Scrollbar area: x=[%d to %d], mouse_x=%d\n",
              d->x + scrollbar_x, d->x + d->w, mouse_x);

          // Check if click is on scrollbar
          if (mouse_x >= d->x + scrollbar_x && mouse_x < d->x + d->w) {
              printf("DEBUG: SCROLLBAR CLICK DETECTED!\n");

              // Calculate scrollbar parameters
              int max_visible = calculate_max_visible_sprites(spm, d->w, d->h);
              int total_sprites = spm->n_total;

              if (total_sprites > max_visible) {
                  // Calculate click position relative to scrollbar track
                  int relative_y = mouse_y - d->y;
                  float click_ratio = (float)relative_y / (d->h - 4);

                  // Calculate new first_sprite based on click position
                  int new_first_sprite = (int)((total_sprites - max_visible) * click_ratio);

                  // Clamp to valid range
                  if (new_first_sprite < 0) new_first_sprite = 0;
                  if (new_first_sprite > total_sprites - max_visible)
                      new_first_sprite = total_sprites - max_visible;

                  spm->first_sprite = new_first_sprite;
                  printf("DEBUG: Scrolling to first_sprite=%d\n", spm->first_sprite);
                  return D_REDRAW;
              }
          }
          else {
              // Original sprite selection code
              printf("DEBUG: Sprite selection click (not scrollbar)\n");
              ix = (mouse_x - 2 - d->x) / spm->sprite_w;
              iy = (mouse_y - 2 - d->y) / spm->sprite_h;
              ic = (iy * spm->n_per_row) + ix + spm->first_sprite;
              if (ic < spm->n_total && ic >= 0)
              {
                  if (d->dp2) {
                      proc = d->dp2;
                      retval = (*proc)(msg, d, ic);
                  }
              }
          }
      }
      return(retval);
      break;

    case MSG_WHEEL:
        // Handle mouse wheel scrolling
        spmp = d->dp;
        spm = *spmp;

        if (spm && spm->bmp) {
            int max_visible = calculate_max_visible_sprites(spm, d->w, d->h);
            int scroll_amount = spm->n_per_row; // Scroll by one row

            if (c > 0) {
                // Scroll up
                spm->first_sprite -= scroll_amount;
                printf("DEBUG: Mouse wheel up - first_sprite=%d\n", spm->first_sprite);
            }
            else {
                // Scroll down
                spm->first_sprite += scroll_amount;
                printf("DEBUG: Mouse wheel down - first_sprite=%d\n", spm->first_sprite);
            }

            clamp_first_sprite(spm, max_visible);
            return D_REDRAW;
        }
        break;

    case MSG_KEY:
        // Handle keyboard scrolling
        spmp = d->dp;
        spm = *spmp;

        if (spm && spm->bmp) {
            int max_visible = calculate_max_visible_sprites(spm, d->w, d->h);
            int scroll_amount;

            switch (c >> 8) {  // Check the scancode
            case KEY_UP:
            case KEY_PGUP:
                // Scroll up
                scroll_amount = spm->n_per_row;
                spm->first_sprite -= scroll_amount;
                printf("DEBUG: Key up - first_sprite=%d\n", spm->first_sprite);
                clamp_first_sprite(spm, max_visible);
                return D_REDRAW;

            case KEY_DOWN:
            case KEY_PGDN:
                // Scroll down
                scroll_amount = spm->n_per_row;
                spm->first_sprite += scroll_amount;
                printf("DEBUG: Key down - first_sprite=%d\n", spm->first_sprite);
                clamp_first_sprite(spm, max_visible);
                return D_REDRAW;

            case KEY_HOME:
                // Scroll to top
                spm->first_sprite = 0;
                printf("DEBUG: Key home - first_sprite=0\n");
                return D_REDRAW;

            case KEY_END:
                // Scroll to bottom
                spm->first_sprite = spm->n_total - max_visible;
                if (spm->first_sprite < 0) spm->first_sprite = 0;
                printf("DEBUG: Key end - first_sprite=%d\n", spm->first_sprite);
                return D_REDRAW;
            }
        }
        break;

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

    case MSG_IDLE:
        bi = d->dp3;
        spmp = d->dp;
        spm = *spmp;

        if (bi->hilite == COLOR_HILITE)
        {
            if (spm != NULL && spm->bmp)
            {
                if ((mouse_x < d->x + d->w - 2)
                    && (mouse_y < d->y + d->h - 2))
                {
                    // determine the x/y position in the structure
                    ix = (mouse_x - 2 - d->x) / spm->sprite_w; // x sprite wide
                    iy = (mouse_y - 2 - d->y) / spm->sprite_h; // y sprite tall
                    ic = (iy * spm->n_per_row) + ix + spm->first_sprite; // sprite number
                    if (ic < spm->n_total && ic >= 0)
                        spm->mouse_over = ic;
                    else
                        spm->mouse_over = spm->n_total + 4242;
                }
                else {
                    spm->mouse_over = spm->n_total + 4242;
                }
            }
        }
        else
            if (spm != NULL)
                spm->mouse_over = spm->n_total + 4242;
        return(retval);
        break;

    case MSG_WANTFOCUS:
        return D_WANTFOCUS;
        break;

    case MSG_CHAR:
        //return D_USED_CHAR;
        bi = d->dp3;
        if (c == (KEY_G << 8)) {         // alt+letter
            ;
        }
        show_mouse(NULL);
        SEND_MESSAGE(d, MSG_DRAW, 0);
        show_mouse(screen);
        break;
    }

    /* always return OK status, since we don't ever need to close
    * the dialog or get the input focus.
    */
    return D_O_K;
}

