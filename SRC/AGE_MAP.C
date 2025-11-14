// map editing tools
// November, 1997 jerry@mail.csh.rit.edu
//   http://www.csh.rit.edu/~jerry
//

#include <stdio.h>
#include <conio.h>
#include <string.h> // for memset()
#include "../INCLUDE/palette.h"
#include "../INCLUDE/fileio.h"
#include "../INCLUDE/maped.h"
#include "../INCLUDE/snap.h"
#include "../INCLUDE/help.h"

int startypos = 0;		// for vertical panning of the map display
int cposx=0;			// position of cursor in map display (x)
int cposy=0;			// position of cursor in map display (y)
int tile_selection=0;		// selection for map editing place
int mapno = 0;			// which map is being edited
char * dataroms = NULL;		// the rom data space
char * editbuf = NULL;		// the edit buffer space (base map data)
char * overbuf = NULL;		// the edit buffer space (overlay map data)
int display_base;		// display the base map (zero if no)
int display_overlay;		// display the overlay (zero if no)
int display_hardcoded;		// display the hardcoded (zero if no)
int display_stretch = 0;	// stretch the display?
signed long snoop_offset = 0l;	// for snooping... offset from structures
struct MAPED_FUNCTIONS * maped_current = NULL; // current function group
MYBITMAP * TempCharL = NULL;	// for the mapcursor function
MYBITMAP * TempCharR = NULL;	// for the mapcursor function

void DecodeMap(void)
{
#if 0
    struct ROMDATA * rd = Driver.RomData;

    if (mapno >= rd->numMaps)
    	return;

    // do the memory allocation as needed...
    if (editbuf){
	free(editbuf);
	editbuf = NULL;
    }
    if (overbuf){
	free(overbuf);
	overbuf = NULL;
    }
    editbuf = (char *) malloc (rd->MapData[mapno].width * rd->MapData[mapno].height);
    if (!editbuf) return;
    overbuf = (char *) malloc (rd->MapData[mapno].width * rd->MapData[mapno].height);
    if (!overbuf){
	free(editbuf);
	editbuf=NULL;
	return;
    }

    // then decode the roms...
    if (maped_current)
	if (maped_current->decode)
	    maped_current->decode();
#endif
}

void EncodeMap(void)
{
#if 0
    if (maped_current)
	if (maped_current->encode)
	    maped_current->encode();
#endif
}


void CreatePlayfield(MYBITMAP * playfield)
{
#if 0
    clear_to_color(playfield, 0); //PALETTEBASE);
    rect(playfield, 0,0, playfield->w-1, playfield->h-1, PAL_GREEN);

    if (maped_current)
    {
	if (display_base)
	    if (maped_current->make_play_base)
		maped_current->make_play_base(playfield);

	if (display_overlay)
	    if (maped_current->make_play_over)
		maped_current->make_play_over(playfield);

	if (display_hardcoded)
	    if (maped_current->make_play_hc)
		maped_current->make_play_hc(playfield);
    }
#endif
}

void display_info(int mode)
{
#if 0
    char text[80];
    struct ROMDATA * rd = Driver.RomData;

    sprintf(text, "Map %d/%d", mapno+1, rd->numMaps);
    textout(screen, font, text, SCREEN_W - 90, 10, PAL_GREY);
    
#ifdef SNOOP_ENABLE
    sprintf(text, "S %08lx", rd->MapData[mapno].StartAddress+snoop_offset);
#else
    sprintf(text, "S %08lx", rd->MapData[mapno].StartAddress);
#endif
    textout(screen, font, text, SCREEN_W - 90, 20, PAL_GREY);

    sprintf(text, "%c %c %c   %c",
                           (display_base)?'B':'b',
                           (display_overlay)?'O':'o',
                           (display_hardcoded)?'H':'h',
                           (display_stretch)?'S':'s'
			   );
    textout(screen, font, text, SCREEN_W - 90, 30, PAL_CYAN);

    if (maped_current)
	if (maped_current->display_info)
	    maped_current->display_info(mode);
#endif
}

int Valid_Overlay_Position(int cxp, int cyp)
{
#if 0
    // return 0 if an invalid position
    if (maped_current)
	if (maped_current->valid_overpos)
	    return maped_current->valid_overpos(cxp, cyp);
    return 1;
#endif
return 0;
}

void ShiftColor(MYBITMAP * bp)
{
#if 0
    // change the bitmap colors to be screen colors, not rom-storage colors
    int x, y;
    for(x=0 ; x<bp->w; x++)
	for(y=0 ; y<bp->h ; y++)
	    putpixel(bp, x,y, (getpixel(bp, x,y) + PALETTEBASE));
#endif
}

void PutTileUnderCursor(int newvalue)
{
#if 0
    struct ROMDATA * rd = Driver.RomData;
    // put in the appropriate checks here for bounds conditions
    // perhaps store these numbers in with the other data in the
    // rom map structure... <shrug>

    if (newvalue > 255) newvalue = 0;
    if (newvalue < 0) newvalue = 255;

    editbuf[cposx + (cposy * rd->MapData[mapno].width)]
	= newvalue & 0xff;

    // now the mirror image for the other half...
    if (newvalue)
	editbuf[(rd->MapData[mapno].width-cposx-1)+
		(cposy * rd->MapData[mapno].width)] = (newvalue & 0xff)^1;
    else
	editbuf[(rd->MapData[mapno].width-cposx-1)+
		(cposy * rd->MapData[mapno].width)] = 0;
#endif
}

unsigned char GetTileUnderCursor(void)
{
#if 0
    return editbuf[cposx + (cposy * Driver.RomData->MapData[mapno].width)];
#endif
return 0;
}

void MapCursor(MYBITMAP * playfield, int x, int y)
{
#if 0
    struct GFXLAYOUT * gl = NULL;
    struct ROMDATA * rd = NULL;
    unsigned char tch;
    unsigned char ochl;
    unsigned char ochr;
    // first re-draw the character at cposx,cposy

    gl = Driver.GfxInfo[GetGfxBank()].GfxLayout;
    rd = Driver.RomData;

    tch = editbuf[cposx + (cposy * rd->MapData[mapno].width)];
    ochl = overbuf[cposx + (cposy * rd->MapData[mapno].width)];
    ochr = overbuf[rd->MapData[mapno].width-1-cposx + (cposy * rd->MapData[mapno].width)];
    if (tch == 0x00 || !display_base)
    {
	clear_to_color(TempCharL, 0); //PALETTEBASE);
	clear_to_color(TempCharR, 0); //PALETTEBASE);
    } else {
	GetImageFromSet(gl, tch, TempCharL);
	GetImageFromSet(gl, tch^1, TempCharR);
    }

    if (display_overlay)
    {
	if (ochl) GetImageFromSet(gl, 0x10, TempCharL);
	if (ochr) GetImageFromSet(gl, 0x10, TempCharR);
    }

    ShiftColor(TempCharL);
    ShiftColor(TempCharR);

    blit(TempCharL, playfield, 0,0, 1+cposx*gl->width,
		1+cposy*gl->height, gl->width, gl->height);
    blit(TempCharR, playfield, 0,0, 1+(rd->MapData[mapno].width-1-cposx)*gl->width,
		1+cposy*gl->height, gl->width, gl->height);

    // re-draw the hardcoded bits
    if (display_hardcoded)
    {
	if (maped_current)
	    if (maped_current->make_play_hc)
		maped_current->make_play_hc(playfield);
    }

    // draw the cursor at position x,y
    cposx=x;
    cposy=y;
    if (timer_counter & 2) // make the cursor blink
	rect(playfield, 1+cposx*gl->width, 1+cposy*gl->height,
                 (cposx+1)*gl->width, (cposy+1)*gl->height, PAL_YELLOW);
#endif
}

void display_tiles(MYBITMAP * tiles)
{
#if 0
    int charno;
    int current_char = GetTileUnderCursor();
    int ypos, xpos;
    int start = 0xb0;
    struct GFXLAYOUT * gl = Driver.GfxInfo[CurrentBank].GfxLayout;
    MYBITMAP * TempChar = create_bitmap(gl->width, gl->height);

    clear_to_color(tiles, PAL_DKGREY);

    xpos = ypos = 0;
    for (charno = start ; charno <= 255 ; charno++)
    {
	GetImageFromSet(gl, (unsigned)charno, TempChar);
	ShiftColor(TempChar);
	blit(TempChar, tiles, 0,0, (xpos)*gl->width+xpos+1,
	      (ypos)*gl->height+ypos+1, gl->width, gl->height);

	// make the selection identifier blink
	if (charno == current_char)
	{
	    rect(tiles, (xpos*gl->width)+xpos, (ypos*gl->height)+ypos,
                 (xpos+1)*gl->width+xpos+1,
		 (ypos+1)*gl->height+ypos+1,
		 (timer_counter & 1)? PAL_YELLOW:PAL_BLACK);
	}
	if (charno == tile_selection)
	{
	    rect(tiles, (xpos*gl->width)+xpos, (ypos*gl->height)+ypos,
                 (xpos+1)*gl->width+xpos+1,
		 (ypos+1)*gl->height+ypos+1,
		 (timer_counter & 1)? PAL_GREEN:PAL_BLACK);
	}
	xpos++;
	if (xpos > 9)
	{
	    xpos = 0;
	    ypos++;
	}
    }

    // just in case it's the null character...
    clear_to_color(TempChar, PALETTEBASE);
    // now display that character
    blit (TempChar, tiles, 0,0, (tiles->w/2)-(gl->width/2),
          tiles->h-gl->height-1, gl->width, gl->height);

    if (current_char < 0xb0)
    {
	rect(tiles,
	    (tiles->w/2)-(gl->width/2)-1,
	    tiles->h-gl->height-2,
	    (tiles->w/2)+(gl->width/2),
	    tiles->h-1,
	    (timer_counter & 1)? PAL_YELLOW:PAL_BLACK);
    }

    if (tile_selection < 0xb0)
    {
	rect(tiles,
	    (tiles->w/2)-(gl->width/2)-1,
	    tiles->h-gl->height-2,
	    (tiles->w/2)+(gl->width/2),
	    tiles->h-1,
	    (timer_counter & 1)? PAL_GREEN:PAL_BLACK);
    }

    destroy_bitmap(TempChar);
    blit(tiles, screen, 0,0,
	    SCREEN_W-tiles->w-2, SCREEN_H-tiles->h-2,
	    tiles->w, tiles->h);
#endif
}


void SelectTile(MYBITMAP * playfield, MYBITMAP * tiles, int cxp, int cyp)
{
#if 0
    int MouseDir = 0, MouseButt = 0;
    int ch = -1;
    int start_ch = GetTileUnderCursor();
    int toc = GetTileUnderCursor(); // tile_selection;

    while (ch != ESCAPE){
	PutTileUnderCursor(toc);
	MapCursor(playfield, cxp, cyp);

	if (display_stretch)
	    stretch_blit(playfield, screen,
	             0, 0, playfield->w-1, playfield->h-1,
		     0, 0, playfield->w-1, 200);
	else
	    blit(playfield, screen, 0,0, 0,startypos, 320,400);

	display_tiles(tiles);
	display_info(MODE_TILE);
    
	MouseDir = mouse_move();
	MouseButt = mouse_butt();
	if (MouseDir || MouseButt){
	    if (MouseButt) {
		ch = ' ';
		MouseButt = 0;
		MouseDir = 0;
		wait_for_button_release();
	    } else
		ch = 0;
	} else {
	    if (kbhit())
		ch = getch();
	    else
		ch = -1;
	    MouseDir = 0;
	    MouseButt = 0;
	}
    
	switch (ch){
	case (-1):
	    break;
	case (ESCAPE):
	case (' '):
	    // select the character, exit
	    ch = ESCAPE;
	    tile_selection = toc;
	    break;

	case ('u'): // scroll map up
	    if (!display_stretch)
	    {
		clear_to_color(screen,PAL_BLACK);
		startypos -=8;
	    }
	    break;
	case ('d'): // scroll map down
	    if (!display_stretch)
	    {
		clear_to_color(screen,PAL_BLACK);
		startypos +=8;
	    }
	    break;

	case ('b'):
	case ('B'):
	    display_base ^= 1;
	    CreatePlayfield(playfield);
	    break;
	case ('o'):
	case ('O'):
	    display_overlay ^= 1;
	    CreatePlayfield(playfield);
	    break;
	case ('h'):
	case ('H'):
	    display_hardcoded ^= 1;
	    CreatePlayfield(playfield);
	    break;

	case ('s'):
	case ('S'):
	    display_stretch ^= 1;
	    clear_to_color(screen,PAL_BLACK);
	    break;

	case (0):
	    // cursor key
	    if (MouseDir)
		ch = ConvertMouseDirection(MouseDir);
	    else
		ch = getch();
	    switch(ch){
		case (HOME):
		    PutTileUnderCursor(start_ch);
		    break;
      case KEY_F1:
         DisplayHelp(HELP_MAP_EDIT);
         break;
         
		case (CURSOR_LEFT):
		    if (toc == 0) toc = 0xff;
		    else {
			toc--;
			if (toc < 0xb0) toc = 0;
		    }
		    PutTileUnderCursor(toc);
		    break;
		case (CURSOR_RIGHT):
		    if (toc == 0) toc = 0xb0;
		    else if (toc == 0xff) toc = 0;
		    else toc++;
		    PutTileUnderCursor(toc);
		    break;
		case (CURSOR_UP):
		    if (toc == 0) toc = 0xfa;
		    else if (toc < 10) toc = 0;
		    else {
			toc -= 10;
			if (toc < 0xb0) toc = 0;
		    }
		    PutTileUnderCursor(toc);
		    break;
		case (CURSOR_DOWN):
		    if (toc > 0xf8) toc = 0x00;
		    else if (toc == 0) toc = 0xb4;
		    else {
			toc += 10;
			if (toc > 0xff) toc = 0;
		    }
		    PutTileUnderCursor(toc);
		    break;

		case (F12):
		    screen_snap();
		    break;
	    }
	    break;
	}
    }
    Save_INI();
#endif
}

void Display_And_Edit_Map(void)
{
#if 0
    struct GFXLAYOUT * gl;
    int oldGfxBank = GetGfxBank();
    struct ROMDATA * rd = Driver.RomData;
    int ch;
    int toc;
    int cxp = cposx;
    int cyp = cposy;
    int MouseDir = 0, MouseButt = 0;
    MYBITMAP * Background;
    MYBITMAP * playfield;
    MYBITMAP * tiles;

    mapno = 0;
    startypos = 0;

    ChangeGfxBank(rd->MapData[mapno].gfx_bank_no);
    gl = Driver.GfxInfo[CurrentBank].GfxLayout;

    playfield = create_bitmap(rd->MapData[mapno].width * gl->width+2,
			      rd->MapData[mapno].height * gl->height+2);
    clear_to_color(playfield, PAL_BLACK);

    tiles = create_bitmap(gl->width * 10+11, gl->height * 9+10);

    Background = create_bitmap(320,200);
    blit(screen, Background, 0,0, 0,0, 320,200);

    // now the screen is setup.

    display_base = 1;
    display_overlay = 1;
    display_hardcoded = 1;

    DecodeMap();
    CreatePlayfield(playfield);

    clear_to_color(screen, PAL_BLACK);

    ch = -1;
    while (ch != ESCAPE)
    {
	display_tiles(tiles);
	display_info(MODE_EDIT);
	if (display_stretch)
	    stretch_blit(playfield, screen,
	             0, 0, playfield->w-1, playfield->h-1,
		     0, 0, playfield->w-1, 200);
	else
	    blit(playfield, screen, 0,0, 0,startypos, 320,400);
	MapCursor(playfield, cxp, cyp);
	
	MouseDir = mouse_move();
	MouseButt = mouse_butt();
	if (!MouseButt && !MouseDir)
	{
	    if (kbhit())
		ch = getch();
	    else
		ch = -1;
	}
	/*
	// i'm not really happy with this functionality...
	if (MouseButt)
	{
	    if (MouseButt & MOUSE_LMB) ch = ' ';
	    if (MouseButt & MOUSE_RMB) ch = 'p';
	    wait_for_button_release();
	}
	*/
	if (MouseButt & MOUSE_LMB) ch = ' ';
	if (MouseButt & MOUSE_RMB) ch = 'z';

	if (MouseDir)
	    ch = 0;

	MapCursor(playfield, cxp, cyp);

	switch (ch){
	case (ESCAPE):
	    if (Dialog_Box("Exit the map editor?",
			   "Strike [esc] again to exit.") == ESCAPE)
		ch = ESCAPE;
	    else
		ch = -1;
	    break;
	case (' '):
	    PutTileUnderCursor(tile_selection);
	    break;
	case ('z'):
	case ('Z'):
	    PutTileUnderCursor(0);
	    break;
	case ('-'):
	    toc = GetTileUnderCursor();
	    if (toc > 0x0b0)
	    	toc--;
	    else if (toc == 0)
		toc = 0xff;
	    else
		toc = 0;
	    PutTileUnderCursor(toc);
	    break;
	case ('='):
	case ('+'):
	    toc = GetTileUnderCursor();
	    if (toc == 0)
		toc = 0x0b0;
	    else
		toc ++;
	    PutTileUnderCursor(toc);
	    break;

	case ('p'):
	case ('P'):
	    if (Valid_Overlay_Position(cxp, cyp))
		overbuf[cxp + (cyp * rd->MapData[mapno].width)] ^= 1;
	    else
		overbuf[cxp + (cyp * rd->MapData[mapno].width)] = 0;

	    break;
	case ('g'):
	case ('G'):
	    // to maintain some consistancy with the main application,
	    // i'm putting this in here... a selector for a piece...
	    SelectTile(playfield, tiles, cxp, cyp);
	    break;

	case ('b'):
	case ('B'):
	    display_base ^= 1;
	    CreatePlayfield(playfield);
	    break;
	case ('o'):
	case ('O'):
	    display_overlay ^= 1;
	    CreatePlayfield(playfield);
	    break;
	case ('h'):
	case ('H'):
	    display_hardcoded ^= 1;
	    CreatePlayfield(playfield);
	    break;

	case ('s'):
	case ('S'):
	    display_stretch ^= 1;
	    clear_to_color(screen,PAL_BLACK);
	    Save_INI();
	    break;

	case ('u'): // scroll map up
	    if (!display_stretch)
	    {
		clear_to_color(screen,PAL_BLACK);
		startypos -=8;
	    }
	    break;
	case ('d'): // scroll map down
	    if (!display_stretch)
	    {
		clear_to_color(screen,PAL_BLACK);
		startypos +=8;
	    }
	    break;
	case ('r'): // reset scroll
	    clear_to_color(screen,PAL_BLACK);
	    startypos =0;
	    break;

	case (','): // palette --
	    if (ModifyPaletteNumber(-1))
	    	SetColours(Driver.GfxInfo[CurrentBank].GfxLayout,0);
	    break;

	case ('.'): // palette ++
	    if (ModifyPaletteNumber( 1))
		SetColours(Driver.GfxInfo[CurrentBank].GfxLayout,0);
	    break;

	case ('3'): // switch resolution to 320x200
	    set_gfx_mode(GFX_VGA,320,200,0,0); // returns 0 on sucess
	    SetColours(Driver.GfxInfo[CurrentBank].GfxLayout,0);
	    SetReservedColours();
	    break;

	case ('6'): // switch resolution to 640x480
	    set_gfx_mode(GFX_VESA1,640,480,0,0); // returns 0 on sucess
	    SetColours(Driver.GfxInfo[CurrentBank].GfxLayout,0);
	    SetReservedColours();
	    break;

	case (0): // extended-character
	    if (MouseDir)
		ch = ConvertMouseDirection(MouseDir);
	    else
		ch = getch();
	    switch(ch){
#ifdef SNOOP_ENABLE
		case (CTRL_KYP_PLUS):
		    snoop_offset += 16;
		    break;
		case (CTRL_KYP_MINUS):
		    snoop_offset -= 16;
		    break;
		case (ALT_KYP_PLUS):
		    snoop_offset++;
		    break;
		case (ALT_KYP_MINUS):
		    snoop_offset--;
		    break;
		case (HOME):
		    snoop_offset = 0;
		    break;
#else
		case (ALT_S): // alt-s for save
		    EncodeMap();
		    SaveRomData();
		    (void)Dialog_Box("Map ROM data saved.",
				     "Hit any key to continue.");
		    break;
#endif

                case (CURSOR_LEFT):
		    if (cxp > 0) cxp--;
		    break;
                case (CURSOR_RIGHT):
		    //if (cxp < (rd->MapData[mapno].width/2)-1) cxp++;
		    if (cxp < (rd->MapData[mapno].width)-1) cxp++;
		    break;
                case (CURSOR_UP):
		    if (cyp > 0) cyp--;
		    break;
                case (CURSOR_DOWN):
		    if (cyp < rd->MapData[mapno].height-1) cyp++;
		    break;

      case KEY_F1:
         DisplayHelp(HELP_MAP_EDIT);
         break;

      case (PAGE_UP):
		    if (mapno < rd->numMaps-1)
		    {
			snoop_offset = 0l;
			mapno++;
			DecodeMap();
			CreatePlayfield(playfield);
		    }
		    break;

		case (PAGE_DOWN):
		    if (mapno > 0)
		    {
			snoop_offset = 0l;
			mapno--;
			DecodeMap();
			CreatePlayfield(playfield);
		    }
		    break;

		case (F12):
		    screen_snap();
		    break;

		default:
		    break;
	    }
	    // now, re-build the map data
#ifdef SNOOP_ENABLE
	    DecodeMap();
	    CreatePlayfield(playfield);
#endif
	    break;

	default:
	    break;

	}
    }


    // now restore the screen
    set_gfx_mode(GFX_VGA,320,200,0,0);
    SetReservedColours();
    SetColours(Driver.GfxInfo[CurrentBank].GfxLayout,0);
    blit(Background, screen, 0,0, 0,0, 320,200);

    destroy_bitmap(Background);
    destroy_bitmap(playfield);
    destroy_bitmap(tiles);
    ChangeGfxBank(oldGfxBank);
#endif
}


void EditMap(void)
{
#if 0
    // look in the driver structure
    if (Driver.RomData == NULL)
    {
	Dialog_Box("Sorry. No ROM data available", "Hit any key to continue");
	return;
    }
    if (Driver.RomData->MapData == NULL)
    {
	Dialog_Box("Sorry. No map data available", "Hit any key to continue");
	return;
    }

    if (!LoadRomData())
    {
	struct GFXLAYOUT * gl = Driver.GfxInfo[GetGfxBank()].GfxLayout;
	if (TempCharL)
	    destroy_bitmap(TempCharL);
	if (TempCharR)
	    destroy_bitmap(TempCharR);

	TempCharL = create_bitmap(gl->width, gl->height);
	TempCharR = create_bitmap(gl->width, gl->height);

	maped_current = Driver.RomData->MapData[mapno].funcs;
	Display_And_Edit_Map();

	if (TempCharL)
	    destroy_bitmap(TempCharL);
	if (TempCharR)
	    destroy_bitmap(TempCharR);
	TempCharL = NULL;
	TempCharR = NULL;
    } else {
	Dialog_Box("Unable to load rom data", "Press any key to continue");
    }
    //sprintf(txt, "Map at 0x%04lx", Driver.RomData->MapData[0].StartAddress);
    //Dialog_Box(txt,txt);
    if (dataroms) {
	free(dataroms);
	dataroms = NULL;
    }

    if (editbuf) {
	free(editbuf);
	editbuf = NULL;
    }

    if (overbuf) {
	free(overbuf);
	overbuf = NULL;
    }
#endif
}
