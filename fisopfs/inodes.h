#ifndef INODES_H
#define INODES_H

#include "./inode.h"
#define INIT_INODES 256

struct DirEntry{
    Inode* inode;
    DirEntry* next;
    
};

struct Inode inodes[INIT_INODES];
struct Inode* free_inode;

void serialize(int fd_out);    // Persona 1
void deserialize(int fd_in);   // Persona 1


// Manejo de inodos
Inode* createInode(Inode* parent, char* path, InodeType tipo); // Persona 4
void deleteInode(Inode* inode); // Persona 4
void getstats(Inode* inode); // Persona 4


// Manejo de directorios
Inode* searchRelative(char* path); // Persona 2
void addChild(Inode* parent, Inode* child); // Persona 2
struct DirEntry* readChildren(Inode* dir); // Persona 2
void freeDir(Inode* dir); // Persona 2


// Manejo de archivos
void writeData(Inode* file, char* buff, int count); // Persona 3
void readData(Inode* file, char* buff_out, int count); // Persona 3
void freeFile(Inode* dir); // Persona 3






#endif
