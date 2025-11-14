// coding.c
//
//  Functions for converting between the sprite palette bitmaps
//  and the format of the memory rom images.
//
//  November, 1998
//   Ivan Mackintosh
//
//  Speedup hacks/rewrite December 1998 Jerry

#include <stdio.h>
#include "../INCLUDE/ALLEGRO.H"
#include "../INCLUDE/sprtplte.h"
#include "../INCLUDE/general.h"
#include "../INCLUDE/gamedesc.h"
#include "../INCLUDE/config.h"
#include "../INCLUDE/coding.h"
#include "../INCLUDE/toolmenu.h"
#include "../INCLUDE/GUIPAL.H"

long nbyte;
int nbit;
int mask;

int GetBit(long startbit)
{
   long nbyte = startbit >> 3;
   int nbit = startbit & 7;    // using an &7 is quicker than an %8
   int mask = 0x80 >> nbit;

   return ((GfxRomData[nbyte] & mask) == mask);
}

// some macros to make everything a little more "standardized"

#define ___setup_plane_info___()\
    for (plane = 0; plane < gbe->planes; plane++)         \
    {                                                     \
	pln[plane] = 1 << (gbe->planes - 1 - plane);      \
    }

#define ___get_sprite_macro___(XXX,YYY)                   \
    result = 0;                                           \
    ww = offs1 + gbe->xoffset[x] + gbe->yoffset[y];       \
    for (plane = 0; plane < gbe->planes; plane++)         \
    {                                                     \
	if (GetBit(ww + gbe->planeoffsets[plane]))        \
	    result |= pln[plane];                         \
    }                                                     \
    putpixel(tbmp, (XXX), (YYY), result + FIRST_USER_COLOR);


void Decode_Normal(int CurrentBank)
{
    GFXBANKEXTRA * gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE * sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int pln[MAX_COL_PLANES];
    long offs1;
    BYTE result = 0;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP * tbmp = create_bitmap(sp->sprite_w,sp->sprite_h);

    clear_to_color(tbmp, FIRST_USER_COLOR);

    ___setup_plane_info___();

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
	offs1 = (gbe->charincrement * spriteno) + StartBit;
	// copy the current character into a bitmap
	for (y = 0; y < sp->sprite_h; y++)
	{
	    for (x = 0; x < sp->sprite_w; x++)
	    {
		___get_sprite_macro___(x,y);
	    }
	}

	// now copy it to the appropriate place in the master gfxbank bitmap
	blit(tbmp, sp->bmp, 0, 0, spriteno * sp->sprite_w, 
	     0, sp->sprite_w, sp->sprite_h);
    }

    destroy_bitmap(tbmp);
}

void Decode_Flip_X(int CurrentBank)
{
    GFXBANKEXTRA * gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE * sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int rx;
    int pln[MAX_COL_PLANES];
    long offs1;
    BYTE result = 0;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP * tbmp = create_bitmap(sp->sprite_w,sp->sprite_h);

    clear_to_color(tbmp, FIRST_USER_COLOR);

    ___setup_plane_info___();

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
	offs1 = (gbe->charincrement * spriteno) + StartBit;
	// copy the current character into a bitmap
	for (y = 0; y < sp->sprite_h; y++)
	{
	    for (x = 0; x < sp->sprite_w; x++)
	    {
		rx = sp->sprite_w-1-x;

		___get_sprite_macro___(rx,y);
	    }
	}

	// now copy it to the appropriate place in the master gfxbank bitmap
	blit(tbmp, sp->bmp, 0, 0, spriteno * sp->sprite_w, 
	     0, sp->sprite_w, sp->sprite_h);
    }

    destroy_bitmap(tbmp);
}

void Decode_Flip_Y(int CurrentBank)
{
    GFXBANKEXTRA * gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE * sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int ry;
    int pln[MAX_COL_PLANES];
    long offs1;
    BYTE result = 0;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP * tbmp = create_bitmap(sp->sprite_w,sp->sprite_h);

    clear_to_color(tbmp, FIRST_USER_COLOR);

    ___setup_plane_info___();

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
	offs1 = (gbe->charincrement * spriteno) + StartBit;
	// copy the current character into a bitmap
	for (y = 0; y < sp->sprite_h; y++)
	{
	    ry = sp->sprite_h-1-y;
	    for (x = 0; x < sp->sprite_w; x++)
	    {
		___get_sprite_macro___(x,ry);
	    }
	}

	// now copy it to the appropriate place in the master gfxbank bitmap
	blit(tbmp, sp->bmp, 0, 0, spriteno * sp->sprite_w, 
	     0, sp->sprite_w, sp->sprite_h);
    }

    destroy_bitmap(tbmp);
}

void Decode_Rotate_90(int CurrentBank)
{
    GFXBANKEXTRA * gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE * sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int ry;
    int pln[MAX_COL_PLANES];
    long offs1;
    BYTE result = 0;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP * tbmp = create_bitmap(sp->sprite_w,sp->sprite_h);

    clear_to_color(tbmp, FIRST_USER_COLOR);

    ___setup_plane_info___();

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
	offs1 = (gbe->charincrement * spriteno) + StartBit;
	// copy the current character into a bitmap
	for (y = 0; y < sp->sprite_h; y++)
	{
	    ry = sp->sprite_h-1-y;
	    for (x = 0; x < sp->sprite_w; x++)
	    {
		___get_sprite_macro___(ry,x);
	    }
	}

	// now copy it to the appropriate place in the master gfxbank bitmap
	blit(tbmp, sp->bmp, 0, 0, spriteno * sp->sprite_w, 
	     0, sp->sprite_w, sp->sprite_h);
    }

    destroy_bitmap(tbmp);
}

void Decode_Rotate_180(int CurrentBank)
{
    GFXBANKEXTRA * gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE * sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int rx,ry;
    int pln[MAX_COL_PLANES];
    long offs1;
    BYTE result = 0;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP * tbmp = create_bitmap(sp->sprite_w,sp->sprite_h);

    clear_to_color(tbmp, FIRST_USER_COLOR);

    ___setup_plane_info___();

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
	offs1 = (gbe->charincrement * spriteno) + StartBit;
	// copy the current character into a bitmap
	for (y = 0; y < sp->sprite_h; y++)
	{
	    ry = sp->sprite_h-1-y;
	    for (x = 0; x < sp->sprite_w; x++)
	    {
		rx = sp->sprite_w-1-x;
		___get_sprite_macro___(rx,ry);
	    }
	}

	// now copy it to the appropriate place in the master gfxbank bitmap
	blit(tbmp, sp->bmp, 0, 0, spriteno * sp->sprite_w, 
	     0, sp->sprite_w, sp->sprite_h);
    }

    destroy_bitmap(tbmp);
}

void Decode_Rotate_270(int CurrentBank)
{
    GFXBANKEXTRA * gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE * sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int rx,ry;
    int pln[MAX_COL_PLANES];
    long offs1;
    BYTE result = 0;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP * tbmp = create_bitmap(sp->sprite_w,sp->sprite_h);

    clear_to_color(tbmp, FIRST_USER_COLOR);

    ___setup_plane_info___();

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
	offs1 = (gbe->charincrement * spriteno) + StartBit;
	// copy the current character into a bitmap
	for (y = 0; y < sp->sprite_h; y++)
	{
	    ry = sp->sprite_h-1-y;
	    for (x = 0; x < sp->sprite_w; x++)
	    {
		rx = sp->sprite_w-1-x;
		___get_sprite_macro___(y,rx);
	    }
	}

	// now copy it to the appropriate place in the master gfxbank bitmap
	blit(tbmp, sp->bmp, 0, 0, spriteno * sp->sprite_w, 
	     0, sp->sprite_w, sp->sprite_h);
    }

    destroy_bitmap(tbmp);
}



void Decode(int CurrentBank)
{
    switch(Orientation)
    {
    case ORIENTATION_FLIP_X:
	Decode_Flip_X(CurrentBank);
	break;
    case ORIENTATION_FLIP_Y:
	Decode_Flip_Y(CurrentBank);
	break;
    case ORIENTATION_ROTATE_180:
    case ORIENTATION_SWAP_XY:
	Decode_Rotate_180(CurrentBank);
	break;
    case ORIENTATION_ROTATE_90:
	Decode_Rotate_90(CurrentBank);
	break;
    case ORIENTATION_ROTATE_270:
	Decode_Rotate_270(CurrentBank);
	break;
    default:
	Decode_Normal(CurrentBank);
	break;
    }
}



void SRC_PutBit(long startbit, int value)
{
    long nbyte = startbit >> 3;
    int nbit = startbit & 7;
    int mask = 0x80 >> nbit;

    GfxRomData[nbyte] &= ~mask;

    if (value)
	GfxRomData[nbyte] |= mask;
}

// some macros to make everything a little more "standardized"

#define PutBit(s,v)                      \
    nbyte = (s) >> 3;                    \
    nbit = (s) & 7;                      \
    mask = 0x80 >> nbit;                 \
    GfxRomData[nbyte] &= ~mask;          \
    if ((v)) GfxRomData[nbyte] |= mask;


#define ___put_sprite_macro___(XXX,YYY)                           \
    ww = offs1 + gbe->xoffset[x] + gbe->yoffset[y];               \
    for (plane = 0; plane < gbe->planes; plane++)                 \
    {                                                             \
	PutBit( ww + gbe->planeoffsets[plane],                    \
		(getpixel(tbmp, (XXX), (YYY)) - FIRST_USER_COLOR) \
		& pln[plane]);                                    \
    }


void Encode_Normal(CurrentBank)
{
    GFXBANKEXTRA * gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE * sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int pln[MAX_COL_PLANES];
    long StartBit = gbe->startaddress * 8;
    MYBITMAP * tbmp = create_bitmap(sp->sprite_w,sp->sprite_h);
    long offs1;

    ___setup_plane_info___();

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
	offs1 = (gbe->charincrement * spriteno) + StartBit;

	// copy the current char into the temp bitmap for rotation
	blit(sp->bmp, tbmp, spriteno * sp->sprite_w, 0, 
	      0, 0, sp->sprite_w, sp->sprite_h);

	for (y = 0; y < sp->sprite_h; y++)
	{
	    for (x = 0; x < sp->sprite_w; x++)
	    {
		___put_sprite_macro___(x,y);
	    }
	}
    }
    destroy_bitmap(tbmp);
}

void Encode_Flip_X(CurrentBank)
{
    GFXBANKEXTRA * gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE * sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int pln[MAX_COL_PLANES];
    int rx;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP * tbmp = create_bitmap(sp->sprite_w,sp->sprite_h);
    long offs1;

    ___setup_plane_info___();

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
	offs1 = (gbe->charincrement * spriteno) + StartBit;

	// copy the current char into the temp bitmap for rotation
	blit(sp->bmp, tbmp, spriteno * sp->sprite_w, 0, 
	      0, 0, sp->sprite_w, sp->sprite_h);

	for (x = 0; x < sp->sprite_w; x++)
	{
	    rx = sp->sprite_w-1-x;
	    for (y = 0; y < sp->sprite_h; y++)
	    {
		___put_sprite_macro___(rx,y);
	    }
	}
    }
    destroy_bitmap(tbmp);
}

void Encode_Flip_Y(CurrentBank)
{
    GFXBANKEXTRA * gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE * sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int pln[MAX_COL_PLANES];
    int ry;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP * tbmp = create_bitmap(sp->sprite_w,sp->sprite_h);
    long offs1;

    ___setup_plane_info___();

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
	offs1 = (gbe->charincrement * spriteno) + StartBit;

	// copy the current char into the temp bitmap for rotation
	blit(sp->bmp, tbmp, spriteno * sp->sprite_w, 0, 
	      0, 0, sp->sprite_w, sp->sprite_h);

	for (y = 0; y < sp->sprite_h; y++)
	{
	    ry = sp->sprite_h-1-y;
	    for (x = 0; x < sp->sprite_w; x++)
	    {
		___put_sprite_macro___(x,ry);
	    }
	}
    }
    destroy_bitmap(tbmp);
}

void Encode_Rotate_90(CurrentBank)
{
    GFXBANKEXTRA * gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE * sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int pln[MAX_COL_PLANES];
    int ry;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP * tbmp = create_bitmap(sp->sprite_w,sp->sprite_h);
    long offs1;

    ___setup_plane_info___();

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
	offs1 = (gbe->charincrement * spriteno) + StartBit;

	// copy the current char into the temp bitmap for rotation
	blit(sp->bmp, tbmp, spriteno * sp->sprite_w, 0, 
	      0, 0, sp->sprite_w, sp->sprite_h);

	for (y = 0; y < sp->sprite_h; y++)
	{
	    ry = sp->sprite_h-1-y;
	    for (x = 0; x < sp->sprite_w; x++)
	    {
		___put_sprite_macro___(ry,x);
	    }
	}
    }
    destroy_bitmap(tbmp);
}

void Encode_Rotate_180(CurrentBank)
{
    GFXBANKEXTRA * gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE * sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int pln[MAX_COL_PLANES];
    int rx,ry;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP * tbmp = create_bitmap(sp->sprite_w,sp->sprite_h);
    long offs1;

    ___setup_plane_info___();

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
	offs1 = (gbe->charincrement * spriteno) + StartBit;

	// copy the current char into the temp bitmap for rotation
	blit(sp->bmp, tbmp, spriteno * sp->sprite_w, 0, 
	      0, 0, sp->sprite_w, sp->sprite_h);

	for (y = 0; y < sp->sprite_h; y++)
	{
	    ry = sp->sprite_h-1-y;
	    for (x = 0; x < sp->sprite_w; x++)
	    {
		rx = sp->sprite_w-1-x;

		___put_sprite_macro___(rx,ry);
	    }
	}
    }
    destroy_bitmap(tbmp);
}

void Encode_Rotate_270(CurrentBank)
{
    GFXBANKEXTRA * gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE * sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int pln[MAX_COL_PLANES];
    int rx;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP * tbmp = create_bitmap(sp->sprite_w,sp->sprite_h);
    long offs1;

    ___setup_plane_info___();

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
	offs1 = (gbe->charincrement * spriteno) + StartBit;

	// copy the current char into the temp bitmap for rotation
	blit(sp->bmp, tbmp, spriteno * sp->sprite_w, 0, 
	      0, 0, sp->sprite_w, sp->sprite_h);

	for (x = 0; x < sp->sprite_w; x++)
	{
	    rx = sp->sprite_w-1-x;
	    for (y = 0; y < sp->sprite_h; y++)
	    {
		___put_sprite_macro___(y,rx);
	    }
	}
    }
    destroy_bitmap(tbmp);
}



void Encode(int CurrentBank)
{
    switch(Orientation)
    {
    case ORIENTATION_FLIP_X:
	Encode_Flip_X(CurrentBank);
	break;
    case ORIENTATION_FLIP_Y:
	Encode_Flip_Y(CurrentBank);
	break;
    case ORIENTATION_ROTATE_180:
    case ORIENTATION_SWAP_XY:
	Encode_Rotate_180(CurrentBank);
	break;
    case ORIENTATION_ROTATE_90:
	Encode_Rotate_90(CurrentBank);
	break;
    case ORIENTATION_ROTATE_270:
	Encode_Rotate_270(CurrentBank);
	break;
    default:
	Encode_Normal(CurrentBank);
	break;
    }
}


// If and old bank is specified (-1 for no old bank) then
// the bitmap for that bank is encoded into the memory roms image
// Once this is complete the bitmap for the new bank is then
// decode from the memory rom image.
void SwitchGraphicsBank(int oldbank, int newbank)
{
    // anything to encode?
    if (oldbank != -1)
	Encode(oldbank);

    // now decode the new one
    Decode(newbank);
}


// localize these definitions
#undef ___setup_plane_info___()
#undef ___get_sprite_macro___(XXX,YYY)
#undef PutBit(s,v)
#undef ___put_sprite_macro___(XXX,YYY)
