// gamedesc.h
//
// All the information required for the driver taken from
// the driver INI file
//
//  October, 1998
//
//   Ivan Mackintosh

#define MAX_GAME_DESCRIPTION  (30)
#define MAX_ROM_NAME (20)

// have we loaded a driver?
extern BOOL GameDriverLoaded;

// the full pathname to the loaded driver
extern char INI_Driver_Path[];

// the full name of the game taken from the Description field of the ini file
extern char GameDescription[MAX_GAME_DESCRIPTION];

extern int NumGfxBanks;

// orientation values - taken from mame src\driver.h
#define	ORIENTATION_DEFAULT		0x00
#define	ORIENTATION_FLIP_X		0x01	/* mirror everything in the X direction */
#define	ORIENTATION_FLIP_Y		0x02	/* mirror everything in the Y direction */
#define ORIENTATION_SWAP_XY		0x04	/* mirror along the top-left/bottom-right diagonal */
#define	ORIENTATION_ROTATE_90	(ORIENTATION_SWAP_XY|ORIENTATION_FLIP_X)	/* rotate clockwise 90 degrees */
#define	ORIENTATION_ROTATE_180	(ORIENTATION_FLIP_X|ORIENTATION_FLIP_Y)	/* rotate 180 degrees */
#define	ORIENTATION_ROTATE_270	(ORIENTATION_SWAP_XY|ORIENTATION_FLIP_Y)	/* rotate counter-clockwise 90 degrees */

extern int Orientation;
extern char ROMDirName[MAX_ROM_NAME];

// array of sprite palettes each one containing a graphics bank and
// associated sprite info
extern SPRITE_PALETTE * GfxBanks;

// addition information required about each graphics bank in order to decode it
typedef struct
{
   long startaddress;
   int planes; // num of bitplanes
   int charincrement; // distance between two consecutive characters/sprites 
   int planeoffsets[8]; // start of every bitplane
   int xoffset[64]; // coordinates of the bit corresponding to the pixel
   int yoffset[64]; // of the given coordinates
} GFXBANKEXTRA;

// array of above type each one containing info about a graphics bank
extern GFXBANKEXTRA * GfxBankExtraInfo;


// the number of graphics roms used by this driver
extern int NumGfxRoms;

typedef struct
{
   long LoadAddress;
   long Size;
   BOOL Alternate;
   char ROMName[MAX_ROM_NAME];
} GFXROM;

// array of above type each one containing information about a rom
extern GFXROM * GfxRoms;

// stores all of the graphics rom data ready to be decoded
// GFXROM struct is the indexes into this memory
extern unsigned char * GfxRomData;

// colour palettes for up to 8 colour planes
#define MAX_COL_PLANES (8)
extern int NumColPalettes[MAX_COL_PLANES];
extern RGB * ColPalettes[MAX_COL_PLANES];

// initialisation to be done on startup


void InitialiseGameDesc(void);