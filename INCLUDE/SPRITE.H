// sprite.h
//
//   sprite palette functions
//
//  November, 1998
//  jerry@mail.csh.rit.edu

extern SPRITE_PALETTE * sprite_bank; // the sprite being used in the display


// internal control variables...
extern int currentGfxBank;


void set_Gfx_bank(int bank);

int sprite_bank_plus(DIALOG *d);
int sprite_bank_minus(DIALOG *d);

int bank_inc(void);
int bank_dec(void);

int sprite_pal_plus(DIALOG *d);
int sprite_pal_minus(DIALOG *d);

int sprite_pal_home(DIALOG *d);

int sprite_return(DIALOG *d);
int sprite_capture(DIALOG *d);

// for the dialog...
int main_sprtplte_callback(int msg, DIALOG * d, int ic);
