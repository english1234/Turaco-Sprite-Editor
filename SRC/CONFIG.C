// config.c
//
//  user configuration stuff
//
//  October, 1998


// NOTE: if any other functions need to use any other .ini files,
//       be sure to use the push_config_state() and pop_config_state()
//       function calls!!!

#include "../INCLUDE/allegro.h"
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <ctype.h>
#include "../INCLUDE/GENERAL.H"
#include "../INCLUDE/util.h"
#include "../INCLUDE/config.h"

#define CONFIG_FILE "turaco.ini"
#define CONFIG_INFO "This file was created from within Turaco."
#define CONFIG_AUTHOR "Turaco"

#define NEWINI_MAJOR 1
#define NEWINI_MINOR 1

int ini_version_major = 0;
int ini_version_minor = 0;

int drv_subdirs = 1;
int troll_magic = 1;
FONT * original_font;

char InitGameName[255];       // as used by load selector

char ROMPath[ROM_PATH_LEN];		// Rom Paths

extern FONT* font;              // Default system font
extern FONT my_new_font;

// config.ini file (application settings file)
//  [System]      general system settings - generally not changed
//  [User]        settings that the user makes each time the visit the app.:w

void Init_INI(void)
{
    unsigned long cver, iniver;

    // initialize the system - create the ini file if necessary
    if (!exists(CONFIG_FILE))
    {
	set_config_file(CONFIG_FILE);
	set_config_int("System", "Version_Major", NEWINI_MAJOR);
	set_config_int("System", "Version_Minor", NEWINI_MINOR);
	set_config_string("System", "Info", CONFIG_INFO);
	set_config_string("System", "Author", CONFIG_AUTHOR);
	set_config_string("System", "ROMPath", DEFAULT_PATH);
	Save_INI();
    } else {
	set_config_file(CONFIG_FILE);
	Load_INI();
    }

    ini_version_major = get_config_int("System", 
				    "Version_Major", ini_version_major);
    ini_version_minor = get_config_int("System", 
				    "Version_Minor", ini_version_minor);

    cver = ((NEWINI_MAJOR)<<16) + NEWINI_MINOR;
    iniver = ((ini_version_major)<<16) + ini_version_minor;

    if (cver < iniver)
    {
	printf("ERROR: The \"%s\" file is newer than TURACO.\n"
	     "       There might be problems when running TURACO!\n"
	     "       Winging it!   Press any key to continue...\n",
	     CONFIG_FILE);
	(void)_getch();
    }
}


void DeInit_INI(void)
{
   // any de-initialization goes here..
}


void Load_INI(void)
{
    int which_font=0;
    // load in all internal bits from the .ini file!
    //strcpy(InitGameName, get_config_string("User", "Default_Driver", "none"));

    gfx_hres = get_config_int("System", "H_Res", gfx_hres);
    gfx_vres = get_config_int("System", "V_Res", gfx_vres);

    drv_subdirs = get_config_on_off("System", "Driver_Subdirs", drv_subdirs);
    troll_magic = get_config_on_off("System", "Troll_Magic", troll_magic);

    // read in the path for the ROMs
    strcpy(ROMPath, get_config_string("System", "ROMPath", DEFAULT_PATH));

    which_font = get_config_on_off("System", "Font", which_font);
    printf("DEBUG: Font setting from config: which_font=%d\n", which_font);

    if (which_font == 0) {
        font = original_font;
        printf("DEBUG: Set font to original_font\n");
    }
    else {
        font = &my_new_font;
        printf("DEBUG: Set font to my_new_font\n");
    }

}

void Save_INI(void)
{
    // save out all internal bits to be stored
    if (font == &my_new_font)
        set_config_int("System", "Font", 1);
    else
        set_config_int("System", "Font", 0);

    set_config_int("System", "H_Res", gfx_hres);
    set_config_int("System", "V_Res", gfx_vres);

    set_config_on_off("System", "Driver_Subdirs", drv_subdirs);

    set_config_on_off("System", "Troll_Magic", troll_magic);

    // write out the path for the ROMs
    set_config_string("System", "ROMPath", ROMPath);

    printf("DEBUG: Save_INI() completed\n");
}