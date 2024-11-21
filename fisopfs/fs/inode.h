#ifndef INODE_H
#define INODE_H

typedef int inode_id_t;
struct Data {
   int size;
};

enum InodeType{
   I_FILE = 1,
   I_DIR = 2
};

// Persona 2
struct Inode{
    char * name;
    enum InodeType tipo;
    struct Data * data;
};




#endif
