// maped.c
//
//  the map editor 
//
//  November, 1998
//  jerry@mail.csh.rit.edu

#include "../INCLUDE/allegro.h"
#include <stdlib.h>

#include "../INCLUDE/general.h"
#include "../INCLUDE/maped.h"
#include "../INCLUDE/iload.h"

extern int alert(const char*, const char*, const char*, const char*, const char*, int, int);

int editors_map(void)
{
    alert("Sorry, The map editor", 
          "is not yet supported.", 
          NULL,
          "&Ok", NULL, 0,0);

    return D_O_K;
}
