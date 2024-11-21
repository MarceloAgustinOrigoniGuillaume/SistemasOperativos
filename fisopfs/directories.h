#ifndef DIRECTORIES_H
#define DIRECTORIES_H

#include "./inode.h"


struct DirData{ // Persona 2
   int size; // Campo de Inode Data*, tiene que estar arriba de todo!
   int capacity;
};


struct DirEntries{
    int count;
    struct Inode* first; // id del inodo? o puede ser algo interno.
};


void serializeDirData(int fd_out, const struct DirData* data_out);    // Persona 2/1?
void deserializeDirData(int fd_in, struct DirData* data);   // Persona 2/1?

// Manejo de directorios
struct Inode* searchRelative(const char* path); // Persona 2
void addChild(struct Inode* parent, struct Inode* child); // Persona 2
void readChildren(struct Inode* dir, struct DirEntries* out); // Persona 2
void freeDir(struct Inode* dir); // Persona 2


#endif
