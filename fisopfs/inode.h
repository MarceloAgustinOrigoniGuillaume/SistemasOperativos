#ifndef INODE_H
#define INODE_H

#include "./data.h"

enum InodeType: int{
   FILE = 1,
   DIR = 2
};

// Persona 2
struct Inode{
    char * name;
    InodeType tipo;
    Data * data;
}




#endif
