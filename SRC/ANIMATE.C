// animate.c
//
//  animate the selections.
//
//  October 1999
//  jerry@mail.csh.rit.edu

//#include "allegro.h"
#include "../INCLUDE/general.h"
#include "../INCLUDE/animview.h"

#include "../INCLUDE/sprtplte.h"
#include "../INCLUDE/sprite.h"
#include "../INCLUDE/gamedesc.h"

#define A_STATE_OFF  (0)
#define A_STATE_ON   (1)

int animate_state = A_STATE_OFF;

int animate_step(void)
{
    if (animate_state == A_STATE_OFF) return D_O_K;

    broadcast_dialog_message (MSG_ANIM_STEP, 0);
    return D_O_K;
}

int animate_force(void)
{
    broadcast_dialog_message (MSG_ANIM_STEP, 0);
    return D_O_K;
}


int animate_toggle(void)
{
    if (animate_state == A_STATE_OFF)
	animate_state = A_STATE_ON;
    else
    {
	animate_state = A_STATE_OFF;
	broadcast_dialog_message(MSG_ANIM_POS_SET, -1);
    }
    return D_REDRAW;
}

int animate_change_playmode(void)
{
    broadcast_dialog_message(MSG_ANIM_MODE, 
                             GfxBanks[currentGfxBank].last_selected);
    return D_REDRAW;
}

int animate_set_point(void)
{
    broadcast_dialog_message(MSG_ANIM_POINT, 
                             GfxBanks[currentGfxBank].last_selected);
    return D_REDRAW;
}
