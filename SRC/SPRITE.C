// sprite.c
//
//   sprite palette functions
//
//  November, 1998
//  jerry@mail.csh.rit.edu

#include <stdio.h>
#include <stdbool.h>
#include "../INCLUDE/general.h"
#include "../INCLUDE/guipal.h"
#include "../INCLUDE/palette.h"
#include "../INCLUDE/sprtplte.h"
#include "../INCLUDE/gamedesc.h"
#include "../INCLUDE/coding.h"
#include "../INCLUDE/util.h"
#include "../INCLUDE/editmode.h"
#include "../INCLUDE/sprite.h"

SPRITE_PALETTE * sprite_bank = NULL; // the sprite being used in the display

int currentGfxBank = 0;

extern MYBITMAP* current_sprite;
extern volatile int mouse_b;    // Mouse button state
extern MYBITMAP* create_bitmap(int width, int height);

void set_Gfx_bank(int bank)
{
    if (GfxBanks == NULL)        return;
    if (bank     >= NumGfxBanks) return;   // bounds check
    if (bank     <  0)           return;

    currentGfxBank = bank;

    sprite_bank = &GfxBanks[bank];
    GfxBanks[bank].flags |= SPRITE_FLAG_NEW; // set the flag

    GfxBanks[bank].first_sprite = 0;
    GfxBanks[bank].n_per_row = 0;
    GfxBanks[bank].n_rows = 0;
    GfxBanks[bank].last_selected = -1;


    // setup the bitmap editor window
    if (current_sprite)
    {
	if (   (current_sprite->w != GfxBanks[bank].sprite_w)
	    || (current_sprite->h != GfxBanks[bank].sprite_h) )
	{
	    destroy_bitmap(current_sprite);
	    current_sprite = NULL;
	}
    }

    if (current_sprite == NULL)
    {
        current_sprite = create_bitmap( GfxBanks[bank].sprite_w, 
					GfxBanks[bank].sprite_h);

        clear_to_color(current_sprite, FIRST_USER_COLOR);
    }
}


int sprite_bank_plus(DIALOG *d)
{
    int sbank = currentGfxBank;

    if (GfxBanks == NULL) return D_O_K;

    set_Gfx_bank(currentGfxBank+1);

    if (sbank != currentGfxBank) {

        busy();
	SwitchGraphicsBank(sbank, currentGfxBank);

	if(GfxBankExtraInfo[sbank].planes != 
	   GfxBankExtraInfo[currentGfxBank].planes)
	    palette_reset(); // fix for bug 03

	not_busy();

	return D_REDRAW;
    } else
	return D_O_K;
}

int bank_inc(void)
{ 
    return sprite_bank_plus(NULL);
}


int sprite_bank_minus(DIALOG *d)
{
    int sbank = currentGfxBank;

    if (GfxBanks == NULL) return D_REDRAW;

    set_Gfx_bank(currentGfxBank-1);

    if (sbank != currentGfxBank) {
        busy();
	SwitchGraphicsBank(sbank, currentGfxBank);

	if(GfxBankExtraInfo[sbank].planes != 
	   GfxBankExtraInfo[currentGfxBank].planes)
	    palette_reset(); // fix for bug 03

	not_busy();

	return D_REDRAW;
    } else
	return D_O_K;
}

int bank_dec(void)
{ 
    return sprite_bank_minus(NULL);
}


int sprite_pal_plus(DIALOG *d)
{
    if (GfxBanks == NULL)
	return D_O_K;

    if (GfxBanks[currentGfxBank].first_sprite < 
	(GfxBanks[currentGfxBank].n_total - GfxBanks[currentGfxBank].n_per_row))
    {
	GfxBanks[currentGfxBank].first_sprite 
	    += GfxBanks[currentGfxBank].n_per_row;
    } else {
	return D_O_K;
    }

    return D_REDRAW;
}


int sprite_pal_minus(DIALOG *d)
{
    if (GfxBanks == NULL)
	return D_O_K;

    if (GfxBanks[currentGfxBank].first_sprite == 0)
	return D_O_K;

    GfxBanks[currentGfxBank].first_sprite -= GfxBanks[currentGfxBank].n_per_row;
    return D_REDRAW;
}

int sprite_pal_home(DIALOG *d)
{
    if (GfxBanks == NULL)
	return D_O_K;

    if (GfxBanks[currentGfxBank].first_sprite == 0)
	return D_O_K;

    GfxBanks[currentGfxBank].first_sprite = 0;
    return D_REDRAW;
}


int sprite_return(DIALOG *d)
{
    if (GameDriverLoaded == FALSE || GfxBanks == NULL)
	return D_O_K;

    if (GfxBanks[currentGfxBank].last_selected >= 0)
	sprite_put(GfxBanks[currentGfxBank].last_selected, 
               current_sprite, &GfxBanks[currentGfxBank]);

    return D_REDRAW;
}

int sprite_capture(DIALOG *d)
{
    if (GameDriverLoaded == FALSE || GfxBanks == NULL)
	return D_O_K;

    if (GfxBanks[currentGfxBank].last_selected >= 0)
	sprite_get(GfxBanks[currentGfxBank].last_selected, 
               current_sprite, &GfxBanks[currentGfxBank]);

    return D_REDRAW;
}


#if 0
// for the editor:

// internal to this module, so no reason to make these public...
#define SEF_NOTHING (0)
#define SEF_EDIT    (1)
#define SEF_REPLACE (2)

int sprite_edit_function = 0;

int sprite_popup_edit(void)
{
    sprite_edit_function = SEF_EDIT;
    return D_O_K;
}
int sprite_popup_replace(void)
{
    sprite_edit_function = SEF_REPLACE;
    return D_O_K;
}
int sprite_popup_cancel(void)
{
    sprite_edit_function = SEF_NOTHING;
    return D_O_K;
}


MENU sprite_popup[] =
{
    {"this is here to keep some space", NULL, NULL, 0, NULL},
    {"",               NULL,                 NULL, 0, NULL},
    {" /\\ &Edit /\\ ",  sprite_popup_edit,    NULL, 0, NULL},
    {"",               NULL,                 NULL, 0, NULL},
    {"   &Cancel",       sprite_popup_cancel,  NULL, 0, NULL},
    {"",               NULL,                 NULL, 0, NULL},
    {" \\/ &Back \\/ ",  sprite_popup_replace, NULL, 0, NULL},
    {NULL,             NULL,                 NULL, 0, NULL},
};


int main_sprtplte_callback(DIALOG * d, int ic)
{
    struct sprite_palette **spmp;
    struct sprite_palette *spm;

    spmp = d->dp;
    spm = *spmp;

    sprite_edit_function = SEF_NOTHING;

    sprintf(sprite_popup[0].text, "Sprite 0x%03x", ic);

    // try to center the popup menu on the cursor...
    do_menu(sprite_popup, mouse_x-56, mouse_y-50);

    switch (sprite_edit_function)
    {
    case (SEF_EDIT): // get
	sprite_get(ic, current_sprite, spm);
	spm->last_selected = ic;
	return D_REDRAW;
	break;

    case (SEF_REPLACE): // put
	sprite_put(ic, current_sprite, spm);
	spm->last_selected = ic; // redundant
	return D_REDRAW;
	break;

    case (SEF_NOTHING):
    default:
	break;
    }
    return(D_O_K);
}

#endif

int main_sprtplte_callback(int msg, DIALOG * d, int ic)
{
    static int last_mb;
    struct sprite_palette **spmp;
    struct sprite_palette *spm;

    spmp = d->dp;
    spm = *spmp;

    spm->last_selected = ic;
    if(msg == MSG_DCLICK)
    {
	if ((last_mb&3) == (mouse_b&3))
	{
	    // make sure it's the same mouse button for a double-click

	    if (last_mb & 2) // right clicked
		sprite_put(ic, current_sprite, spm); // right d-click -> replace
	    else
		sprite_get(ic, current_sprite, spm); // left d-click -> grab
		
	    return(D_REDRAW);
	}
    } else {
	last_mb = mouse_b;
    }

    return(D_O_K);
}
