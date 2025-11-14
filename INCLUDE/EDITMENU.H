// editmenu.h
//
//  edit menu functions
//
//  September, 1998
//  jerry@mail.csh.rit.edu

extern MYBITMAP * clipboard;

int edit_undo(void);
int edit_redo(void);
int edit_copy_sprite(void);
int edit_paste_sprite(void);
int edit_save_pcx(void);
int edit_load_pcx(void);
int edit_preferences(void);
