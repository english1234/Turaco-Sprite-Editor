// ====================================================================
// snapshot utility
//
// January, 1998 jerry@mail.csh.rit.edu
//   http://www.csh.rit.edu/~jerry
// ====================================================================
//

#include <../allegro.h>  

// if you have the Jpeg library by Eric Vannier, available
// from  http://www-inf.enst.fr/~vannier/
// and you want to save snapshots as jpegs instead of .pcx files
// then #define this next line instead of #undef'ing it.
#undef  SNAP_JPEG

// the directory which gets the snapshots:
#define SNAPDIR  "snapshot"

// the name of the file that gets created ie:
// "snap"  ->  "snap0001.pcx", "snap0002.pcx", etc...
#define SNAPBASE "snap"

void snap(MYBITMAP *bmp, RGB * pal);

void screen_snap(void);
