// inidriv.h
//
//  Ini driver loader functions...
//
//  October, 1998
//
//   Ivan Mackintosh

BOOL LoadDriver(char * INIFileName);
void FreeDriver(void);
BOOL SaveDriver(void);
void SwitchGraphicsBank(int oldbank, int newbank);
void MakePatch(void);
void MergeFiles(char * ROMName, long size);
void ApplyPatch(void);
void SaveAllPalettes();
