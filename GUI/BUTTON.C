// button.c
//
//  my replacement button proc
//
//  September, 1998
//  jerry@mail.csh.rit.edu

#include "../INCLUDE/ALLEGRO.H"
#include "../INCLUDE/general.h"

extern int d_button_proc(int msg, DIALOG* d, int c);

int button_dp2_proc(int msg, DIALOG *d, int c)
{
    int ret;
    int (*proc)(DIALOG *dl);

    /* call the parent object */
    ret = d_button_proc(msg, d, c);

    /* trap the close return value */
    if (ret==D_CLOSE)
    {
    	// call the sub-function
	if (d->dp2)
	{
	    proc = d->dp2;
	    return (*proc)(d);
	} else 
	    return D_REDRAW;
    }
    return ret;
}
