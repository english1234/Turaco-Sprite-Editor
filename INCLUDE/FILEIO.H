// fileio.h
//
//  The file io functions
//
//  October, 1998
//  jerry@mail.csh.rit.edu


// menu options
int file_game(void);
int file_save_gfx(void);
int file_save_c_source(void);
int file_revert(void);
int file_genpatch(void);
int file_applypatch(void);
int file_exit(void);

// pass in a path to a driver, and if it worked, then 
// GameDriverLoaded will be true
void try_loading_the_driver(char * drivername);

// unload and free all driver memory, 
// and return the system to a known state
void unload_driver(void);

