#ifndef INODE_H
#define INODE_H

#include <time.h>

typedef int inode_id_t;

struct Data {
   int size;
};

enum InodeType{
   I_FILE = 1,
   I_DIR = 2
};

// Persona 2
struct Inode {
    inode_id_t id;          
    char *name;             
    enum InodeType type;    
    struct Data *data;      
    int permissions;        
    int blocks;      
    time_t created;      
    time_t modified;     
    time_t last_access;
    struct Inode* next;     
};


#endif
