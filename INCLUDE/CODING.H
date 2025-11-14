// coding.h
//
//  Functions for converting between the sprite palette bitmaps
//  and the format of the memory rom images.
//
//  November, 1998
//
//   Ivan Mackintosh


// encodes the oldbank and decodes the new. if the oldbank
// is -1 then no encoding done.
void SwitchGraphicsBank(int oldbank, int newbank);


// for before we save....

#define Commit_Graphics_Bank() \
	SwitchGraphicsBank(currentGfxBank, currentGfxBank);
