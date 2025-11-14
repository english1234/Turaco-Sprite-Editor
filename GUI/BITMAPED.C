// bitmaped.c
//
//  bitmap editor UI module for allegro
//
//  September, 1998
//  jerry@mail.csh.rit.edu
//
//  Re-written 5-7 March 1999
//  jerry@absynth.com

// see the .h file for what data is to be displayed in what fields

#include "../INCLUDE/ALLEGRO.H"
#include <stdio.h>
#include <memory.h>
#include "../INCLUDE/general.h"
#include "../INCLUDE/toolmenu.h"
#include "../INCLUDE/util.h"
#include "../INCLUDE/bitmaped.h"
#include "../INCLUDE/GUIPAL.H"

extern MYBITMAP* screen;          // Main display bitmap
extern FONT* font;              // Default system font
extern volatile int mouse_x;
extern volatile int mouse_y;
extern volatile int mouse_b;

extern MYBITMAP* create_bitmap(int width, int height);
extern void text_mode(int mode);

void bitmaped_free_tables(struct _bitmap_editor_internals* bi)
{
	if (bi->xscal)
		free(bi->xscal);
	if (bi->yscal)
		free(bi->yscal);
	bi->xscal = NULL;
	bi->yscal = NULL;
}

void bitmaped_generate_tables(MYBITMAP* bmp,
	DIALOG* d,
	struct _bitmap_editor_internals* bi)
{
	int c;

	if (bi == NULL) return;

	bitmaped_free_tables(bi);

	// Allocate tables for the canvas size (dialog size minus 4 for border)
	int canvas_width = d->w - 4;
	int canvas_height = d->h - 4;

	bi->xscal = (int*)malloc(sizeof(int) * canvas_width);
	bi->yscal = (int*)malloc(sizeof(int) * canvas_height);

	if (bi->xscal == NULL || bi->yscal == NULL)
	{
		bitmaped_free_tables(bi);
		return;
	}

	memset(bi->xscal, 0, (sizeof(int) * canvas_width));
	memset(bi->yscal, 0, (sizeof(int) * canvas_height));

//	printf("Bitmap: %dx%d, Canvas: %dx%d\n", bmp->w, bmp->h, canvas_width, canvas_height);

	// FIXED: Calculate the scale factor (how many canvas pixels per sprite pixel)
	float pixels_per_sprite_x = (float)canvas_width / bmp->w;  // Should be 5.0 for 80/16
	float pixels_per_sprite_y = (float)canvas_height / bmp->h; // Should be 5.0 for 80/16

//	printf("Pixels per sprite: %.1fx%.1f\n", pixels_per_sprite_x, pixels_per_sprite_y);

	// FIXED: Generate proper scaling tables
	// Each sprite pixel should occupy a block of canvas pixels
	for (int canvas_x = 0; canvas_x < canvas_width; canvas_x++) {
		// Map canvas coordinate to sprite coordinate
		bi->xscal[canvas_x] = (int)(canvas_x / pixels_per_sprite_x);
		if (bi->xscal[canvas_x] >= bmp->w) bi->xscal[canvas_x] = bmp->w - 1;
		if (bi->xscal[canvas_x] < 0) bi->xscal[canvas_x] = 0;
	}

	for (int canvas_y = 0; canvas_y < canvas_height; canvas_y++) {
		// Map canvas coordinate to sprite coordinate  
		bi->yscal[canvas_y] = (int)(canvas_y / pixels_per_sprite_y);
		if (bi->yscal[canvas_y] >= bmp->h) bi->yscal[canvas_y] = bmp->h - 1;
		if (bi->yscal[canvas_y] < 0) bi->yscal[canvas_y] = 0;
	}
}

void bitmaped_draw_grid(MYBITMAP* bmp, struct _bitmap_editor_internals* bi)
{
	int c;
	int last = -2;

	for (c = 0; c < (bmp->h) - 4; c++)
	{
		if (bi->xscal[c] != last)
		{
			last = bi->xscal[c];
			line(bmp, c + 2, 2, c + 2, bmp->w - 3, GUI_FORE);
		}
	}

	last = -2;
	for (c = 0; c < (bmp->w) - 4; c++)
	{
		if (bi->yscal[c] != last)
		{
			last = bi->yscal[c];
			line(bmp, 2, c + 2, bmp->w - 3, c + 2, GUI_FORE);
		}
	}
}

void bitmaped_draw_bitmap(MYBITMAP* src, MYBITMAP* dst, struct _bitmap_editor_internals* bi)
{
	if (bi == NULL || bi->xscal == NULL || bi->yscal == NULL) {
		stretch_blit(src, dst, 0, 0, src->w, src->h, 2, 2, dst->w - 4, dst->h - 4);
		return;
	}

	int canvas_width = dst->w - 4;
	int canvas_height = dst->h - 4;

	// Clear the destination
	clear_to_color(dst, GUI_BACK);

	// Simple block drawing: each sprite pixel becomes a block in the canvas
	int block_width = canvas_width / src->w;  // Should be 5
	int block_height = canvas_height / src->h; // Should be 5

	for (int sprite_y = 0; sprite_y < src->h; sprite_y++) {
		for (int sprite_x = 0; sprite_x < src->w; sprite_x++) {
			int color = getpixel(src, sprite_x, sprite_y);

			// Calculate the canvas block for this sprite pixel
			int canvas_start_x = sprite_x * block_width;
			int canvas_start_y = sprite_y * block_height;
			int canvas_end_x = canvas_start_x + block_width;
			int canvas_end_y = canvas_start_y + block_height;

			// Fill the entire block with the sprite pixel color
			for (int cy = canvas_start_y; cy < canvas_end_y; cy++) {
				for (int cx = canvas_start_x; cx < canvas_end_x; cx++) {
					int dest_x = cx + 2;  // Account for border
					int dest_y = cy + 2;
					if (dest_x < dst->w && dest_y < dst->h) {
						putpixel(dst, dest_x, dest_y, color);
					}
				}
			}
		}
	}
}


/* custom dialog procedure for the bitmap editor object */
int bitmap_editor_proc(int msg, DIALOG* d, int c)
{
	MYBITMAP* temp;
	MYBITMAP** temp1;
	MYBITMAP* temp2;
	MYBITMAP* temp3;
	struct _bitmap_editor_internals* bi;
	int x, y;
	int lastx = -1, lasty = -1;
	int ret = D_O_K;
	void (*proc)(DIALOG * dl, int x_pos, int y_pos, int mouse);

	/* process the message */
	switch (msg) {

		/* initialise when we get a start message */
	case MSG_START:
		if (d->dp) {
			MYBITMAP** temp1 = d->dp;
			MYBITMAP* temp2 = *temp1;
			if (temp2) {
				printf("Bitmap: %dx%d\n", temp2->w, temp2->h);
			}
			else {
				printf("Bitmap pointer is NULL!\n");
			}
		}

		d->dp3 = malloc(sizeof(struct _bitmap_editor_internals));
		bi = d->dp3;
		bi->hilite = COLOR_LOLITE;
		bi->grid = 1;
		bi->xscal = NULL;
		bi->yscal = NULL;
		bi->old_w = -2;
		bi->old_h = -2;

		if (d->dp != NULL) {
			MYBITMAP** temp1 = d->dp;
			MYBITMAP* temp2 = *temp1;
			if (temp2 != NULL) {
				bitmaped_generate_tables(temp2, d, bi);
			}
		}
		break;

		/* shutdown when we get an end message */
	case MSG_END:
		bi = d->dp3;
		if (bi)
		{
			bitmaped_free_tables(bi);
			free(bi);
			d->dp3 = NULL;
		}
		break;

		/* draw in response to draw messages */
	case MSG_DRAW:
		bi = d->dp3;

		temp = create_bitmap(d->w, d->h);
		clear_to_color(temp, 3); //d->bg);

		if (d->dp == NULL)
		{
			text_mode(-1);
			textout_centre(temp, font, "??", (d->w) / 2, (d->h) / 2, GUI_FORE);
		}
		else {
			temp1 = d->dp;  // pointer to the bitmap to draw
			temp2 = *temp1; // this is the bitmap to draw
			if (temp2 == NULL)
			{
				text_mode(-1);
				textout_centre(temp, font, "No Gfx!", temp->w / 2, temp->h / 2, GUI_FORE);
			}
			else {
				if (d->dp != NULL) {
					MYBITMAP** temp1 = d->dp;
					MYBITMAP* temp2 = *temp1;
					if (temp2 != NULL) {
						bitmaped_generate_tables(temp2, d, bi);
					}
				}
				// if it's a different size than before, update the tables...
				if ((bi->old_w != temp2->w) ||
					(bi->old_h != temp2->h))
				{
					bi->old_w = temp2->w;
					bi->old_h = temp2->h;
					(void)SEND_MESSAGE(d, MSG_SIZE_CHANGE, 0);
					bi = d->dp3;
				}

				// draw the bitmap here...
				bitmaped_draw_bitmap(temp2, temp, bi);

				// and the grid if necessary
				if (bi->grid)
					bitmaped_draw_grid(temp, bi);

				// now draw out the small version of the bitmap...
				if (d->d1 != -1 && d->d2 != -1)
				{
					temp3 = create_bitmap(temp2->w + 2, temp2->h + 2);
					if (temp3)
					{
						clear_to_color(temp3, GUI_MID);
						blit(temp2, temp3, 0, 0, 1, 1, temp2->w, temp2->h);
						border_3d(temp3, 0, 0, temp3->w, temp3->h, 0);
						blit(temp3, screen, 0, 0, d->d1, d->d2, temp3->w, temp3->h);
						destroy_bitmap(temp3);
					}
				}
			}
		}

		// the highlight/selection boxen
		box_3d(temp, temp->w, temp->h,
			(bi->hilite == COLOR_HILITE) ? 1 : 0,
			(bi->hilite == COLOR_HILITE) ? 1 : 0
		);

		// In the MSG_DRAW case after blitting to screen, add:
// and copy it to the screen
		blit(temp, screen, 0, 0, d->x, d->y, d->w, d->h);

		// Verify the screen has the updated pixel
		int screen_x = d->x + 22; // Approximate position for sprite(2,1)
		int screen_y = d->y + 12;

		destroy_bitmap(temp);
		break;

	case MSG_CLICK:
		temp1 = d->dp;
		temp2 = *temp1;
		bi = d->dp3;

		if (!temp2) {
			printf("ERROR: No bitmap to edit!\n");
			return D_O_K;
		}

		// Constrain mouse (expanded range to account for cursor offset)
		set_mouse_range(d->x + 2, d->y + 2, d->x + d->w + 10, d->y + d->h + 10);

		Uint32 click_start_time = SDL_GetTicks();
		const Uint32 MAX_CLICK_DURATION = 10000;

		while (mouse_b) {
			if (SDL_GetTicks() - click_start_time > MAX_CLICK_DURATION) {
				printf("WARNING: Click loop timeout\n");
				break;
			}

			// Process events
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				update_mouse_state(&event);
				if (event.type == SDL_QUIT) goto click_loop_exit;
			}

			// Calculate canvas coordinates
			// Subtract dialog position and border (2px), then adjust for cursor offset
			int canvas_x = mouse_x - d->x - 10;
			int canvas_y = mouse_y - d->y - 6;

			// Bounds check
			if (canvas_x >= 0 && canvas_x < d->w - 4 && canvas_y >= 0 && canvas_y < d->h - 4) {
				// SIMPLIFIED: Use direct calculation instead of scaling tables
				int block_width = (d->w - 4) / temp2->w;  // Should be 5
				int block_height = (d->h - 4) / temp2->h; // Should be 5

				x = canvas_x / block_width;
				y = canvas_y / block_height;

				// Bounds check for sprite
				if (x >= 0 && x < temp2->w && y >= 0 && y < temp2->h) {
					if (lastx != x || lasty != y) {

						// Call painting callback
						if (d->dp2) {
							proc = d->dp2;
							(*proc)(d, x, y, mouse_b);
						}

						// Force redraw
						show_mouse(NULL);
						SEND_MESSAGE(d, MSG_DRAW, 0);
						SDL_Flip();

						show_mouse(screen);

						lastx = x;
						lasty = y;
					}
					else {
						printf(" - Same position\n");
					}
				}
				else {
					printf(" - Out of sprite bounds\n");
				}
			}
			else {
				printf(" - Out of canvas bounds\n");
			}

			SDL_Delay(50);
		}

	click_loop_exit:
		set_mouse_range(0, 0, SCREEN_W - 1, SCREEN_H - 1);
		break;

	case MSG_DCLICK:
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

	case MSG_WANTFOCUS:
		return D_WANTFOCUS;
		break;

	case MSG_CHAR:
		temp1 = d->dp;  // pointer to the bitmap to draw
		temp2 = *temp1; // this is the bitmap to draw

		bi = d->dp3;
		if (c == (KEY_X << 8)) {
			do_tool(TOOLS_HORIZ_FLIP, temp2, 0, 0);
			ret = D_USED_CHAR;
		}
		else if (c == (KEY_Y << 8)) {
			do_tool(TOOLS_VERT_FLIP, temp2, 0, 0);
			ret = D_USED_CHAR;

		}
		else if (c == (KEY_Z << 8)) {
			do_tool(TOOLS_CW_ROTATE, temp2, 0, 0);
			ret = D_USED_CHAR;
		}
		else if (c == (KEY_W << 8)) {
			do_tool(TOOLS_CW_ROTATE_45, temp2, 0, 0);
			ret = D_USED_CHAR;

		}
		else if (c == (KEY_UP << 8)) { // alt+letter
			do_tool(TOOLS_WRAP_UP, temp2, 0, 0);
			ret = D_USED_CHAR;
		}
		else if (c == (KEY_DOWN << 8)) { // alt+letter
			do_tool(TOOLS_WRAP_DOWN, temp2, 0, 0);
			ret = D_USED_CHAR;
		}
		else if (c == (KEY_LEFT << 8)) { // alt+letter
			do_tool(TOOLS_WRAP_LEFT, temp2, 0, 0);
			ret = D_USED_CHAR;
		}
		else if (c == (KEY_RIGHT << 8)) { // alt+letter
			do_tool(TOOLS_WRAP_RIGHT, temp2, 0, 0);
			ret = D_USED_CHAR;

		}
		else if (c == (KEY_K << 8)) { // alt+letter
			do_tool(TOOLS_CLEAR_BITMAP, temp2, 0, 0);
			ret = D_USED_CHAR;

		}
		else if (c == (KEY_G << 8)) {         // alt+letter
			bi->grid ^= 1;
			ret = D_USED_CHAR;
		}
		show_mouse(NULL);
		SEND_MESSAGE(d, MSG_DRAW, 0);
		show_mouse(screen);
		break;

	case MSG_SIZE_CHANGE:
		temp1 = d->dp;  // pointer to the bitmap to draw
		temp2 = *temp1; // this is the bitmap to draw
		bi = d->dp3;

		if ((bi != NULL) && (temp2 != NULL) && (d != NULL))
		{
			bitmaped_generate_tables(temp2, d, bi);
		}
		break;
	}

	/* always return OK status, since we don't ever need to close
	* the dialog or get the input focus.
	*/
	return ret;
}