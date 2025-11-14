// animview.h
//
//  animation viewer UI module for allegro
//
//  September, 1998
//  jerry@mail.csh.rit.edu

#include "../INCLUDE/ALLEGRO.H"

/* custom dialog procedure for the bitmap editor object */
int animview_proc(int msg, DIALOG *d, int c);
// dp is a pointer to a pointer to a bitmap.

// new messages
#define MSG_ANIM_STEP  (MSG_USER+100)  

#define MSG_ANIM_POINT (MSG_USER+101)  
#define MSG_ANIM_MODE  (MSG_USER+102)  
