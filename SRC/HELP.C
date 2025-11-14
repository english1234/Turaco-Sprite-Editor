// help.c
//
//  the help functions
//
//  November/December, 1998
//  jerry@mail.csh.rit.edu

#include "../INCLUDE/allegro.h"
#include <stdio.h>
#include "../INCLUDE/general.h"

//
/// HELP MENU FUNCTIONS
//

char* text_file_buffer = NULL;

DIALOG text_file_dialog[] =
{
	/* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)
		 (key) (flags)  (d1)  (d2)  (dp) */
		 // { d_clear_proc,      0, 0, 0, 0, GUI_FORE, GUI_BACK,    
		  { d_shadow_box_proc, 0,    0,    315,  236,  0,    0,    0,    0,       0,    0,    NULL },
		  { d_ctext_proc,      152,  6,    1,    1,    0,    0,    0,    0,       0,    0,    "                                              " },
		  { d_textbox_proc,      5,  15,   305 ,  195,    0,    0,    0,    0,       0,    0,  NULL },

		  { d_button_proc,     231,  216,  80,   16,   0,    0,    'o',  D_EXIT,  0,    0,    "&OK" },
		   { NULL },
};

void display_text_file(char* title, char* filename, int startpos)
{
	FILE* fp;

	// what makes this next block of code unique is that i wrote it,
	// compiled it with no errors, and it worked properly the first
	// time it was run...  unheard-of, really...
	// i've since simplified it by removing references to sys/stat 
	// for file sizes, and replaced them with allegro's file_size()

	text_file_dialog[2].d2 = startpos;
	if (exists(filename))
	{
		if (text_file_buffer) {
			free(text_file_buffer);
			text_file_buffer = NULL;  // Prevent double-free
		}

		//sprintf(text_file_dialog[1].dp, title);
		text_file_dialog[1].dp = title;

		text_file_buffer = (char*)malloc(file_size(filename));
		if (text_file_buffer)
		{
			fp = fopen(filename, "rb");
			if (fp)
			{
				fread(text_file_buffer, 1, file_size(filename), fp);
				fclose(fp);

				text_file_dialog[2].dp = text_file_buffer;

				centre_dialog(text_file_dialog);
				set_dialog_color(text_file_dialog, GUI_FORE, GUI_BACK);
				do_dialog(text_file_dialog, 2);

				if (text_file_buffer) {
					free(text_file_buffer);
					text_file_buffer = NULL;  // Prevent double-free
				}
			}
			else {
				alert("File access error!", "Could not load file:",
					filename,
					"Continue", NULL, 0, 0);
			}
		}
		else {
			alert("Memory allocation error!", "Could not load file:",
				filename,
				"Continue", NULL, 0, 0);
		}
	}
	else {
		alert("Could not find file:", filename, "",
			"Continue", NULL, 0, 0);
	}

}

int help_general(void)
{
	// let it work if the text files are in the TXT\ directory
	if (exists("TXT\\README.TXT"))
		display_text_file("Readme Text File", "TXT\\README.TXT", 0);
	else
		display_text_file("Readme Text File", "README.TXT", 0);

	return D_REDRAW;
}

int help_new(void)
{
	if (exists("TXT\\WHATSNEW.TXT"))
		display_text_file("What's New Text File", "TXT\\WHATSNEW.TXT", 0);
	else
		display_text_file("What's New Text File", "WHATSNEW.TXT", 0);

	return D_REDRAW;
}

int help_about(void)
{
	char line1[127];

	sprintf(line1, "%s [%s] by", title_text, title_date);

	alert(line1, "Scott \"Jerry\" Lawrence & Ivan Mackintosh", "Robin Champion (Windows version 2025)",
		"Okay", NULL,
		0, 0);

	return D_O_K;
}