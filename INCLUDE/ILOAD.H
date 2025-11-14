// iload.h
//
//  The rom driver loader functions
//
//  December, 1998
//  jerry@mail.csh.rit.edu

#define MAP_TYPE_NONE    (0)
#define MAP_TYPE_PACMAN  (1)

typedef struct map_data{
    int map_type;
    int Gfx_Bank;
    long width;
    long height;
    long Start_Address;
    long Overlay_Address;
    long Overlay_Count;   // eventually - OverlayCountAddress
    int hc[8]; // hardcoded info - eventually hc_address[]
} MAP_DATA;

extern BOOL DataRomsLoaded;

extern BOOL MapDataAvailable;
extern BOOL TextDataAvailable;

void Data_Driver_Sanity_Check(void);

void Load_Data_Roms(void);
void Free_Data_Roms(void);

void Get_Map_Driver_Data(void);
void Get_Text_Driver_Data(void);
