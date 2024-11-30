#ifndef INODE_H
#define INODE_H

#include <time.h>
#include <sys/types.h>

typedef int inode_id_t;

#define NOT_DEFINED_BLOCK -1
//struct Data {
//   int size;
//};

enum InodeType{
   I_FILE = 1,
   I_DIR = 2
};

// Persona 2
struct Inode {
    inode_id_t id;          
    char *name;             
    enum InodeType type;    
    int permissions;        
    time_t created;      
    time_t modified;     
    time_t last_access;
    struct Inode* next_free;  
    
    //struct Data *data;      
    int blocks;      
    int first_block;      
    int size_bytes;      
    
};

struct DirEntries{
     int size;
     inode_id_t * first;
};


int setNewName(struct Inode* inode, const char* name);

#endif
