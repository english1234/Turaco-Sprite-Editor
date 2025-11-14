// fileio.c
//
//  The file io functions
//
//  October, 1998
//  jerry@mail.csh.rit.edu

#include "../INCLUDE/allegro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../INCLUDE/general.h"
#include "../INCLUDE/drivsel.h"
#include "../INCLUDE/INIDriv.h"
#include "../INCLUDE/sprtplte.h"
#include "../INCLUDE/guipal.h"
#include "../INCLUDE/gamedesc.h"
#include "../INCLUDE/palette.h"
#include "../INCLUDE/editmode.h"
#include "../INCLUDE/sprite.h"
#include "../INCLUDE/coding.h"

extern MYBITMAP* current_sprite;
extern MYBITMAP* create_bitmap(int width, int height);
extern int alert(const char*, const char*, const char*, const char*, const char*, int, int);

void unload_driver(void)
{
    FreeDriver();
    InitialiseGameDesc();
    free(sprite_bank);
    sprite_bank = NULL; 

    destroy_bitmap(current_sprite);
    current_sprite = create_bitmap(ED_DEF_SIZE,ED_DEF_SIZE);
    clear_to_color(current_sprite, FIRST_USER_COLOR);
}

void try_loading_the_driver(char * drivername)
{
    unload_driver();

    if (LoadDriver(drivername) == TRUE)
    {
	// setup the sprite palette & stuff..
	set_Gfx_bank(0);

	// load in the palette
	Init_Palette();
    } else {
	// we should do this here...
	unload_driver();
    }
}

//
/// FILE MENU FUNCTIONS
//

// file_game  -  menu option
int file_game(void)
{
    if(GameDriverLoaded)
    {
	if (alert("ARE YOU SURE?", 
	          "You can't undo this!", 
	          "(even if we had an undo)", 
	      "&Continue", "Cancel", 0, 0) == 2)
	      return D_O_K;
    }

    // unload the driver here...
    unload_driver();

    game_driver_selector();

    busy();

    if (strlen(selected_filename))
    {
	try_loading_the_driver(selected_filename);
	if(!GameDriverLoaded)
	{
	    not_busy();
	    alert("Error loading driver:", selected_filename, "Sorry.", 
	          "&Okay", NULL, 'O', 0);

	    unload_driver();
	}
    }

    not_busy();

    return D_REDRAW;
}

int file_revert(void)
{
    // re-load the data from the roms

    if(GameDriverLoaded)
    {
	if (alert("ARE YOU SURE?", 
	          "You can't undo this!", 
	          "(even if we had an undo)", 
	      "&Continue", "Cancel", 0, 0) == 1)
	{
	    busy();
	    unload_driver();

	    if (LoadDriver(INI_Driver_Path) == TRUE)
	    {
		// setup the sprite palette & stuff..
		set_Gfx_bank(0);

		// load in the palette
		Init_Palette();
	    }

	    not_busy();
	    return D_REDRAW;
	}

    } else {
	alert("Cannot revert graphics!", "No romdata loaded!", "Sorry.", 
	      "&Okay", NULL, 'O', 0);
    }

    return D_O_K;
}

int file_exit(void)
{
    if (GameDriverLoaded)
    {
	if (alert("Are you sure you want", 
	          "to exit and abandon any", 
	          "changes you've made?", 
		  "&Cancel", "E&xit", 'c', 'x') == 1)
	    return(D_O_K);
	else
	{
#ifndef NO_FADE
	    fade_out(FADE_SPEED);
#endif
	    return(D_EXIT);
	}
    } 
#ifndef NO_FADE
    fade_out(FADE_SPEED);
#endif
    return(D_EXIT); // note: if this is D_O_K, then you need to load a driver
                    //       in order to quit.  yow!
}

int file_save_gfx(void)
{
    // save out the graphics data to the roms
    if (GameDriverLoaded)
    {
	busy();

	// fix for bug 07 -- perhaps we should just Encode() here?
	Commit_Graphics_Bank();

	if (SaveDriver() == TRUE)
	{
	    not_busy();
	    alert("", "Save complete", NULL, "&Okay", NULL, 'O', 0);
	}
	not_busy();
    } else {
	alert("Cannot save graphics!", "No romdata loaded!", "Sorry.",
	"&Okay", NULL, 'O', 0);
    }

    return D_O_K;
}

int file_save_c_source(void)
{
    char the_path[255];
    char newname[32];
    char numbre[32];
    char *pos;
    FILE * fp;
    long fpos;
    long lpos;

    if (!GameDriverLoaded)
    {
	alert("Cannot save source!", "No romdata loaded!", "Sorry.", 
		"&Okay", NULL, 'O', 0);
	return D_REDRAW;
    }
    
    strncpy(the_path, get_config_string("System", "C_Source", "."), 255);
    put_backslash(the_path);

    // generate a possible filename
    strcpy(newname, ROMDirName);
    newname[6] = '\0';
    sprintf(numbre, "%02d", currentGfxBank+1);
    strcat(newname, numbre);
    strcat(the_path, newname);
    strcat(the_path, ".c");

    if (file_select("Enter A filename:", the_path, "C;CPP") )
    {
	Commit_Graphics_Bank();
	// save it out now 
	fp = fopen(the_path, "w");
	if (fp)
	{
	    lpos = 9999;
	    fprintf(fp, "char data[] =\n{");
	    for(fpos = GfxBankExtraInfo[currentGfxBank].startaddress;
		fpos < GfxBankExtraInfo[currentGfxBank].startaddress+
		       (GfxBankExtraInfo[currentGfxBank].charincrement>>3)
		       * GfxBanks[currentGfxBank].n_total ;
		fpos ++)
	    {
		if (lpos >= 8)
		{
		    lpos = 0;
		    fprintf(fp, "\n   ");
		}

		// write it out here...
		fprintf(fp, " 0x%02x,", GfxRomData[fpos]);
		lpos++;
	    }
	    fprintf(fp, "\n};\n");
	    fclose(fp);

	} else {
	    alert("Sorry, file error:", the_path, "Save aborted", 
	      "&Okay", NULL, 'O', 0);
	}

	pos = get_filename(the_path);
	*pos = '\0';
	set_config_string("System", "C_Source", the_path);
    }
    return(D_REDRAW);
}

int file_genpatch(void)
{
    // generate the patch file from the data changed...
    if(GameDriverLoaded)
    {
	busy();

	// fix for bug 07 -- perhaps we should just Encode() here?
	Commit_Graphics_Bank();

	MakePatch();
	not_busy();
	alert("", "Patch generation complete", NULL, "&Okay", NULL, 'O', 0);

    } else {
	alert("Cannot generate patch!", "No romdata loaded!", "Sorry.",
	"&Okay", NULL, 'O', 0);
    }
    return D_O_K;
}

int file_applypatch(void)
{
    // generate the patch file from the data changed...
    if(GameDriverLoaded)
    {
	busy();

	ApplyPatch();

	set_Gfx_bank(0);

	not_busy();

	alert("", "Patch application complete", NULL, "&Okay", NULL, 'O', 0);
	return D_REDRAW;
    } else {
	alert("Cannot apply patch!", "No romdata loaded!", "Sorry.",
	"&Okay", NULL, 'O', 0);
    }
    return D_O_K;
}