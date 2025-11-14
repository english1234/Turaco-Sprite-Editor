// drivsel.h
//
//  The file selector functions
//
//  November, 1998
//  jerry@mail.csh.rit.edu


extern char selected_filename[];

void game_driver_selector(void);



// for the driver directories...

extern char ** dirlist;
extern int ndirs;
void create_dir_list(void);
void destroy_dir_list(void);
int file_exists(const char* filename, int attrib, int* aret);
int for_each_file(const char* pattern, int attrib,
	void (*callback)(char*, int, int), int param);