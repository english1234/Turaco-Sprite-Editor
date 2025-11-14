// map editing tools
// November, December 1997 jerry@mail.csh.rit.edu
//   http://www.csh.rit.edu/~jerry
//

// many many thanks go to David Caldwell, for help
//  with the basic map organization information for PacMan Maps!
// David_Caldwell@indigita.com  ->  http://www.indigita.com/david/arcade/
//
// many many thanks also go to M.C.Silvius for mspacman map info,
//  and motivation... :)
// M.C.Silvius@caiw.nl -> http://www.caiw.nl/~msilvius
//
// pacman maze starts in rom 6j at location 0x436 (1078)
// it is stored as wall information with RLE spaces for pellets
// starting in the upper right, going down, then to the left.
// only 1/2 of the maze is stored, the other half is constructed from
// a mirror image of the first, and the wall bits are the same as
// from the other side XOR 1
//
// ie:
//   +----+
//   |....|
//   \.++./ 	would be stored as:
//   /.++.\ 		+|/\<+-4--1++1-
//   >....<
//   +----+ 	(if the char value < 0xb0, it's a space)
//
//  The map playfield is 28 characters horizontal, by 32 characters vertical
//  (but only 14 horizontal are stored, the other 14 are the mirrored image)
//
//  Now my (jerry's) research:
//     Pellet data is stored at 0x5b5 thru 0x6a4 in rom 6j as RLE of
//  spaces, starting in the same location as the map background data
//  above.  they are stored as a series of offsets from the previous
//  position.  ie: the real pacman pellet data is stored as:
//  62 1 1 2 1 1, so the leftmost column and 1 space are skipped,
//  then 1 pellet, then 1 pellet, then a skip&1 pellet, and 1 pellet, etc.
//  if the above ascii map is the entire map, then it would be encoded
//  as:  8 1 1 1 3 3  maybe...  I'm not sure where the power pellets
//  are stored... <shrug> ... so they get blit out to the screen in
//  hardcoded positions (mspac puts them in different positions on
//  screen for each of her four maps)
//
//  A pacman map MUST have 240 pellets in it for the game to function
//  properly.  Any more than that, and it over-writes rom data.  Any
//  fewer than that and the board NEVER ENDS!
//  There are a few locations in the map in which the pellets will not
//  show (making them impossible to get) they are: (9,12)-(18,12) and
//  (11,18)-(16,18). I account for these in the 'p' case in the main
//  map edit loop.  These locations are where "Player n" and "Ready!"
//  appear near the ghost bin.
//
//  if the map data itself is larger than 0x17e bytes, it will over-write
//  other data in the roms, distorting the map data entirely.
//
//  past the end of a "short" map, which uses less than 0x17e bytes, the
//  romspace should be filled with 0's to be sure that no garbage gets
//  put out to the screen.
//
// As far as Ms.Pac-Man, i know that the 4 maps are in the BOOT6/U6 rom
// that's about all i know at this point.
//
// other info (from David Caldwell)
// - the number of pellets is stored at 0x08E2 in rom 6e
//   however, changing this value breaks the program.  This needs
//   to be looked into further
// - starting at 0x0EFD and going thru 0xF3b in rom 6e is the table for
//   fruit sprite numbers, colors, and point value.  (byte, byte, word),
//   16 of them...  some kind of editor for this would be keen.
//   struct fruits {
//       unsigned char which_sprite;
//       unsigned char color_of_sprite;
//       unsigned word point value
//   };
//
// Info from M.C. Silvius:
//  see additional documents on the web at:
//   http://www.csh.rit.edu/~jerry/arcade/pacman/
//   all this data and screen organization are available there...
//
// Many thanks to Yuri Software for making HEdit32 (Hex editor for WinNT)
// it helped me a lot in figuring out the format for the pellets

#include <stdio.h>
#include <conio.h>
#include <string.h> // for memset()
#include "../INCLUDE/allegro.h"
#if 0
#include "../drivers.h"
#include "../age.h"  
#include "../palette.h"
#include "../fileio.h"
#include "../mouse.h"
#include "../tools.h"
#include "../maped.h"
#endif

void Maped_Decode_PacMan(void)
{
#if 0
    // load map number "mapno" from the roms into the edit buffer
    long rompos;
    int spacecount;
    int chl,chr,cho;
    int done = 0;
    int cx,cy;
    unsigned char romchar;
    struct ROMDATA * rd = Driver.RomData;

    memset(overbuf, 0x00, (rd->MapData[mapno].width * rd->MapData[mapno].height) );

    // now, decode the rom into the editbuf

    rompos = rd->MapData[mapno].StartAddress;
    rompos += snoop_offset;
    if (rompos < 0) rompos = 0;
    spacecount = 0;
    cx=cy=0;

    while (!done)
    {
	if (!spacecount) {
	    romchar = dataroms[rompos] & 0xff;
	    rompos++;
	    if (romchar < 0xb0)
	    	spacecount = romchar-1;
	}

	if (spacecount) {
	    spacecount--;
	    // it's a RLE for space
	    chl = 0;
	    chr = 0;
	} else {
	    // it's a real character
	    chl = romchar^1;
	    chr = romchar;
	}
	editbuf[cx + (cy * rd->MapData[mapno].width)] = chl;
	editbuf[(rd->MapData[mapno].width-cx-1)+ (cy * rd->MapData[mapno].width)] = chr;

	cy++;
	if (cy >= rd->MapData[mapno].height)
	{
	    cy = 0;
	    cx++;
	    /*
	    // i thought this might be a fix for pirnaha, and some
	    // slightly invalid pac maps, but i'm not sure how the
	    // centre column is stored... it's odd.  load piranha
	    // and notice that the centre column is not there, however
	    // it is under pacman. <shrug>
	    if (cx==13)//account for the centre column
		rompos = rd->MapData[mapno].StartAddress
		         + snoop_offset + 0x015b;
	    */
	}
	if (cx >= rd->MapData[mapno].width/2)
	    done = 1;
    }
    //mapsize = rompos-rd->MapData[mapno].StartAddress;

    // now decode the overlay data (pellots)
    done = 0;
    cx=cy=0;
    spacecount = 0;
    rompos = rd->MapData[mapno].OverlayAddr;
    while (!done)
    {
	if (!spacecount) {
	    romchar = dataroms[rompos] & 0xff;
	    if (rompos == rd->MapData[mapno].OverlayAddr)
		romchar -= 63; //kludge to get it working
	    rompos++;
	    spacecount = romchar-1;
	    cho = 0x00;
	} else {
	    spacecount--;
	    cho = 0;
	}
	if (spacecount == 0)
	    cho = 1;

	if (rompos > rd->MapData[mapno].OverlayAddr+0x6a4-0x5b5)
	    done = 1;

	overbuf[(rd->MapData[mapno].width-cx-1)+ (cy * rd->MapData[mapno].width)] = cho;

	cy++;
	if (cy >= rd->MapData[mapno].height)
	{
	    cy = 0;
	    cx++;
	}
	if (cx > rd->MapData[mapno].width-1)
	    done = 1;
    }
    //ovlsize = rompos-rd->MapData[mapno].OverlayAddr;
#endif
}

void Maped_Encode_PacMan(void)
{
#if 0
    // save map number "mapno" from the edit buffer into the roms
    long rompos;
    int spacecount;
    int chr;
    int done = 0;
    int cx,cy;
    struct ROMDATA * rd = Driver.RomData;

    if (!editbuf) return;

    // now, encode the editbuf back into the rom
    rompos = rd->MapData[mapno].StartAddress;
    if (rompos < 0) rompos = 0;
    spacecount = 0;
    cx=cy=0;

    while (!done)
    {
	chr = 0x00ff & editbuf[(rd->MapData[mapno].width-cx-1)+ (cy * rd->MapData[mapno].width)];
	if (chr < 0xb0) {
	    spacecount++;
	} else {
	    if (spacecount){
		dataroms[rompos] = (unsigned char) ((spacecount+1) & 0xff);
		spacecount = 0;
		rompos++;
	    }
	    dataroms[rompos] = (unsigned char) chr & 0xff;
	    rompos++;
	}
	// check to see if we might over-write other bits of the rom...
	if (rompos > rd->MapData[mapno].StartAddress+0x17d)
	    done = 1;

	cy++;
	if (cy >= rd->MapData[mapno].height)
	{
	    cy = 0;
	    cx++;
	}
	if (cx >= rd->MapData[mapno].width/2)
	    done = 1;
    }

    // write out 0's to the rom in case there isn't enough map data
    // to fill the romspace.
    while (rompos < rd->MapData[mapno].StartAddress+0x17d)
    {
	// if we end up in here, the map is short
	dataroms[rompos] = 0x00;
	rompos++;
    }


    // now encode the overlay
    done = 0;
    cx=cy=0;
    rompos = rd->MapData[mapno].OverlayAddr;
    spacecount = 64; // kludge factor
    while (!done)
    {
	chr = overbuf[(rd->MapData[mapno].width-cx-1)+ (cy * rd->MapData[mapno].width)];
	if (chr) {
	    dataroms[rompos] = (char)spacecount;
	    rompos++;
	    spacecount = 1;
	} else {
	    spacecount++;
	}
	if (rompos > rd->MapData[mapno].OverlayAddr+0x6a4-0x5b5)
	    done = 1; // eliminate writing over other rom data

	cy++;
	if (cy >= rd->MapData[mapno].height)
	{
	    cy = 0;
	    cx++;
	}
	if (cx > rd->MapData[mapno].width-1)
	    done = 1;
    }
    while (rompos < rd->MapData[mapno].OverlayAddr+0x6a4-0x5b5)
    {
	// if we end up in here, there's a problem with the map.
	dataroms[rompos] = 0x00;
	rompos++;
    }
#endif
}

void Maped_Playfield_Base_PacMan(MYBITMAP * playfield)
{
#if 0
    // convert the editbuffer into a displayable map image
    struct GFXLAYOUT * gl = Driver.GfxInfo[CurrentBank].GfxLayout;
    struct ROMDATA * rd = Driver.RomData;
    MYBITMAP * TempChar = create_bitmap(gl->width, gl->height);
    unsigned char tch;
    int cx,cy;

    for(cx = 0; cx < rd->MapData[mapno].width ; cx++)
	for(cy = 0; cy < rd->MapData[mapno].height ; cy++)
	{
	    tch = editbuf[cx + (cy * rd->MapData[mapno].width)];
	    if (tch < 0xb0)
		clear_to_color(TempChar, 0); //Fix - Jerry 2/16
	    else 
		GetImageFromSet(gl, tch, TempChar);

	    ShiftColor(TempChar);

	    blit(TempChar, playfield, 0,0,
			cx*gl->width+1, cy*gl->height+1,
			gl->width, gl->height);
	}

    destroy_bitmap(TempChar);
#endif
}

void Maped_Playfield_Over_PacMan(MYBITMAP * playfield)
{
#if 0
    // convert the editbuffer into a displayable map image
    struct GFXLAYOUT * gl = Driver.GfxInfo[CurrentBank].GfxLayout;
    struct ROMDATA * rd = Driver.RomData;
    MYBITMAP * TempChar = create_bitmap(gl->width, gl->height);
    unsigned char tch;
    int cx,cy;

    GetImageFromSet(gl, 0x10, TempChar);
    ShiftColor(TempChar);

    for(cx = 0; cx < rd->MapData[mapno].width ; cx++)
	for(cy = 0; cy < rd->MapData[mapno].height ; cy++)
	{
	    tch = overbuf[cx + (cy * rd->MapData[mapno].width)];
	    if (tch)
	    {
		blit(TempChar, playfield, 0,0,
			cx*gl->width+1, cy*gl->height+1,
			gl->width, gl->height);
	    }
	}

    destroy_bitmap(TempChar);
#endif
}


void Maped_Playfield_HC_PacMan(MYBITMAP * playfield)
{
#if 0
    int count;
    int x,y;
    struct GFXLAYOUT * gl = Driver.GfxInfo[CurrentBank].GfxLayout;
    MYBITMAP * TempChar = create_bitmap(gl->width, gl->height);

    GetImageFromSet(gl, 0x14, TempChar);
    ShiftColor(TempChar);

    for (count = 0 ;
         count < Driver.RomData->MapData[mapno].numHardcoded ;
	 count ++)
	{
	    x= Driver.RomData->MapData[mapno].hardcoded[2*count];
	    y= Driver.RomData->MapData[mapno].hardcoded[(2*count)+1];
	    blit(TempChar, playfield, 0,0, (x)*gl->width+1, (y)*gl->height+1, gl->width, gl->height);
	}


    destroy_bitmap(TempChar);
#endif
}

int Maped_TotalOverlay_PacMan(void)
{
#if 0
    int total = 0;
    int pos = 0;
    struct ROMDATA * rd = Driver.RomData;

    for (pos = 0 ; pos < (rd->MapData[mapno].width*rd->MapData[mapno].height); pos++)
	if (overbuf[pos]) total++;
    return total;
#endif
return 0;
}

int Maped_TotalMap_PacMan(void)
{
#if 0
    // this is basically the encode without outputting to the rom...
    struct ROMDATA * rd = Driver.RomData;
    int total = 0;
    int cx=0, cy=0;
    int chr;
    int done = 0;
    int spacecount=0;

    while (!done)
    {
	chr = 0x00ff & editbuf[(rd->MapData[mapno].width-cx-1)+ (cy * rd->MapData[mapno].width)];
	if (chr < 0xb0) {
	    spacecount++;
	} else {
	    if (spacecount){
		spacecount = 0;
		total++;
	    }
	    total++;
	}

	cy++;
	if (cy >= rd->MapData[mapno].height) {
	    cy = 0;
	    cx++;
	}
	if (cx >= rd->MapData[mapno].width/2)
	    done = 1;
    }

    return total;
#endif
return 0;
}

void Maped_Info_PacMan(int mode)
{
#if 0
    char text[80];
    int max;
#ifdef SNOOP_ENABLE
    struct ROMDATA * rd = Driver.RomData;
#endif

    sprintf(text, "Size %04x", Maped_TotalMap_PacMan());
    if (Maped_TotalMap_PacMan() > 0x17d)
	textout(screen, font, text, SCREEN_W - 90, 40, PAL_RED);
    else
	textout(screen, font, text, SCREEN_W - 90, 40, PAL_GREEN);

    max = Driver.RomData->MapData[mapno].OverlayCount;
    sprintf(text, "#p %3d/%3d", Maped_TotalOverlay_PacMan(), max);

    if (Maped_TotalOverlay_PacMan() != max)
	textout(screen, font, text, SCREEN_W - 90, 50, PAL_RED);
    else
	textout(screen, font, text, SCREEN_W - 90, 50, PAL_GREEN);

#ifdef SNOOP_ENABLE
    sprintf(text, "O %08lx", snoop_offset);
    textout(screen, font, text, SCREEN_W - 90, 60, PAL_GREY);

    sprintf(text, "T %08lx", snoop_offset + rd->MapData[mapno].StartAddress);
    textout(screen, font, text, SCREEN_W - 90, 70, PAL_GREY);
#endif

    if (mode == MODE_EDIT)
	textout(screen, font, "M: Map Edit", SCREEN_W - 90, 95, PAL_YELLOW);
    else
	textout(screen, font, "M: Tile Sel", SCREEN_W - 90, 95, PAL_GREEN);

    sprintf(text, " 0x%02x ", GetTileUnderCursor());
    textout_centre(screen, font, text, SCREEN_W - 45, 105, PAL_LTGREY);
#endif
}

int Maped_Overlay_Pos_Valid_PacMan(int cxp, int cyp)
{
#if 0
    // return 0 if an invalid position
    if (  (cyp == 12 && cxp >=9  && cxp <=18)    // "Player 1"
       || (cyp == 18 && cxp >=11 && cxp <=16) )  // "Ready!"
	return 0;
    return 1;
#endif
return 0;
}


#if 0
struct MAPED_FUNCTIONS PacMan_Functions =
{
    &Maped_Decode_PacMan,
    &Maped_Encode_PacMan,
    &Maped_Playfield_Base_PacMan,
    &Maped_Playfield_Over_PacMan,
    &Maped_Playfield_HC_PacMan,
    &Maped_Overlay_Pos_Valid_PacMan,
    &Maped_Info_PacMan,
};
#endif
